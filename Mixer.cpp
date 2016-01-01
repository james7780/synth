/// Implementation of CMixer class - mix voices to output
#include <stdio.h>
#include <math.h>
#include "common.h"


#define PI	3.14159265358979f

///////////////////////////////////////////////////////////////////////
/// CMixer class - mix all voices into one output
///////////////////////////////////////////////////////////////////////

CMixer::CMixer()
	:	m_masterVolume(0.75f),
		m_LFOWaveform(WT_SINE),
		m_LFOFreq(1.0f),
		m_LFODepth(0.5f),
		m_LPFilter(SAMPLERATE, SAMPLERATE / 2),
		m_reverb(100.0f, 0.6f),
		m_bendRatio(1.0f),
		m_targetBendRatio(1.0f)
{
	// Voices are initialised by their constructor
	for (int i = 0; i < MIXBUFFER_LENGTH; i++)
		m_mixBuffer[i] = 0.0f;
}


CMixer::~CMixer()
{
}

#define LIMIT1	20000.0f
#define LIMIT2	26000.0f
#define RATIO1	0.7f
#define RATIO2	0.2f

/// Homemade compression algorithm
/// Mostly avoids clipping with up to 16 voices mixed
static float CompressSample(float value)
{
	// Compress this sample to range -32767 to +32767
	if (value > LIMIT1)
		{
		float d = value - LIMIT1;
		value = LIMIT1 + (d * RATIO1);
		if (value > LIMIT2)
			{
			d = value - LIMIT2;
			value = LIMIT2 + (d * RATIO2);
			}
		}
	else if (value < -LIMIT1)
		{
		float d = value + LIMIT1;
		value = -LIMIT1 + (d * RATIO1);
		if (value < -LIMIT2)
			{
			d = value + LIMIT2;
			value = -LIMIT2 + (d * RATIO2);
			}
		}

	// Check
	if (value > 32700)
		printf("overflow: %.0f\n", value);
	else if (value < -32700)
		printf("underflow: %.0f\n", value);

	return value;
}

/// Fill the output buffer using voice data
/// @param buffer			The output buffer (16-bit mono)
/// @param numSamples		The number of 16-bit samples to output
/// @param sampleRate		The output sample rate (usually 44100 or 48000)
void CMixer::FillBuffer(short* buffer, int numSamples, int sampleRate)
{
	static float LFOWavePos = 0.0f;
	
	const float bufferPeriodMS = (float)(1000 * numSamples / sampleRate);

	// Update smoothing bend value
	float d = (m_targetBendRatio - m_bendRatio) * 0.3f;
	m_bendRatio += d;

	// Calc LFO level
	int LFOOffset = (int)LFOWavePos % NOMINAL_WAVE_SIZE;
	float LFOLevel = m_LFODepth * m_LFOWaveform.m_samples[LFOOffset];
	
//printf("LL %.3f\n", LFOLevel);

	// Zero mix buffer
	for (int i = 0; i < MIXBUFFER_LENGTH; i++)
		m_mixBuffer[i] = 0.0f;

	// Mix all active voices
	for (int n = 0; n < NUM_VOICES; n++)
		{
		// Only process this voice if it is active
		if (m_voice[n].m_wavePos > -1)
			{
			// Add this voice output to mix buffer, unscaled
			float envVol = m_voice[n].GetEnvelopeLevel(ET_VOLUME);
			float envLFOVol = m_voice[n].GetLFOEnvelopeLevel(ET_VOLUME);
			float outputVol = m_masterVolume * m_voice[n].m_volume * envVol + (LFOLevel * envLFOVol);
			float envPitch = m_voice[n].GetEnvelopeLevel(ET_PITCH);
			float envLFOPitch = m_voice[n].GetLFOEnvelopeLevel(ET_PITCH);
			float outputFreq = m_voice[n].m_freq / sampleRate;
			outputFreq *= (1.0f + envPitch + LFOLevel * envLFOPitch);
			outputFreq *= m_bendRatio;
			float x = (float)m_voice[n].m_wavePos;
			unsigned int waveformLength = m_voice[n].m_waveform.m_numSamples;
#define PHASESHIFT
#ifdef PHASESHIFT
			float duty = m_voice[n].m_osc->m_duty;
			float dy1 = 0.5f / duty;			// slope before duty point
			float dy2 = 0.5f / (1.0f - duty);	// slope after duty point
			int L1 = (int)(duty * waveformLength);
			int halfWFL = waveformLength / 2;
#endif
			for (int i = 0; i < numSamples; i++)
				{
				int offset = (int)x % waveformLength;
#ifdef PHASESHIFT
				// Adjust (interpolate) offset according to duty
				if (offset < L1)
					offset = offset * dy1;
				else
					offset = halfWFL + (offset - L1) * dy2;
				float s1 = outputVol * m_voice[n].m_waveform.m_samples[offset];

#else
				float s1 = outputVol * m_voice[n].m_waveform.m_samples[offset];
#endif
				m_mixBuffer[i] += s1; // + s2;

				x += outputFreq * waveformLength;
				}
			m_voice[n].m_wavePos = (int)x % waveformLength;
			m_voice[n].m_elapsedTime += bufferPeriodMS;
			}
		}

	// Update LFO
	LFOWavePos += m_LFOFreq * (bufferPeriodMS / 1000.0f) * NOMINAL_WAVE_SIZE;
	if (LFOWavePos > NOMINAL_WAVE_SIZE)
		LFOWavePos -= NOMINAL_WAVE_SIZE;

	// Apply reverb
	m_reverb.Apply(m_mixBuffer, numSamples, sampleRate);

	/// Apply LP filter
	m_LPFilter.Apply(m_mixBuffer, numSamples);

	// Now write our mix buffer to the output buffer
	// Note: We compress output to avoid distortion
	float scale = 8000; //16000;
	for(int i = 0; i < numSamples; i++)
		{
		// compress
		float value = CompressSample(m_mixBuffer[i] * scale);
		//float value = mixBuffer[i] * scale;
		buffer[i] = (short)value;
		}

}

/// Get the first free voice we can find
/// @return				Pointer to first free voice , or NULL
CVoice *CMixer::GetFreeVoice()
{
	for (int n = 0; n < NUM_VOICES; n++)
		{
		if (-1 == m_voice[n].m_wavePos)
			{
			return &m_voice[n];
			}
		}

	return 0;		// NULL
}

/// Get the voice which is playing the specified note
CVoice *CMixer::GetVoiceFromNote(char note)
{
	for (int n = 0; n < NUM_VOICES; n++)
		{
		if (m_voice[n].m_note == note)
			{
			return &m_voice[n];
			}
		}

	return 0;		// NULL
}

