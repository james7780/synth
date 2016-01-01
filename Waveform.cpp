#include "Waveform.h"
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

// TODO : "duty cycle" setting to phase distort any
//        wave
CWaveform::CWaveform(WAVETYPE type, float duty)
	:	m_type(type),
		m_samples(NULL),
		m_numSamples(0)
{
	SetWaveform(type, duty);
}


CWaveform::~CWaveform()
{
}

/// Init the wave data to the specified waveform
/// @param[in] type		Waveform type
/// @param[in] duty		Waveform duty cycle (0.0 to 1.0) (default = 0.5)
void CWaveform::SetWaveform(WAVETYPE type, float duty)
{
	m_type = type;
	switch (type)
		{
		case WT_SQUARE:
			MakeSquareWave(duty);
			break;
		case WT_SINE:
			MakeSineWave(duty);
			break;
		case WT_SAW1:
			MakeSaw1Wave(duty);
			break;
		case WT_SAW2:
			MakeSaw2Wave(duty);
			break;
		case WT_TRIANGLE:
			MakeTriangleWave(duty);
			break;
		case WT_NOISE:
			MakeNoiseWave();
			break;
		default:
			MakeSquareWave();
			break;
		}
}

/// The one and only noise waveform
static float NOISEWAVE[NOISE_WAVE_SIZE] = { 0.0f };

/// @param[in] duty		Waveform duty cycle (0.0 to 1.0) (default = 0.5)
void CWaveform::MakeSineWave(float duty)
{
	// Make a sine wave
	if (m_samples != NOISEWAVE)
		delete m_samples;
	m_samples = new float[NOMINAL_WAVE_SIZE];
	m_numSamples = NOMINAL_WAVE_SIZE;
	for (int i = 0; i < NOMINAL_WAVE_SIZE; i++)
		{
		m_samples[i] = sinf(2.0f * M_PI * (float)i / NOMINAL_WAVE_SIZE);
		}
}

/// @param[in] duty		Waveform duty cycle (0.0 to 1.0) (default = 0.5)
void CWaveform::MakeSquareWave(float duty)
{
	// Make a (semi) square wave
	if (m_samples != NOISEWAVE)
		delete m_samples;
	m_samples = new float[NOMINAL_WAVE_SIZE];
	m_numSamples = NOMINAL_WAVE_SIZE;
	for (int i = 0; i < NOMINAL_WAVE_SIZE; i++)
		{
		float a = 10.0f * sinf(2.0f * M_PI * (float)i / NOMINAL_WAVE_SIZE);
		a = tanh(a);			// limit to range -1 to 1
		m_samples[i] = a;
		}
}

/// @param[in] duty		Waveform duty cycle (0.0 to 1.0) (default = 0.5)
void CWaveform::MakeSaw1Wave(float duty)
{
	// Make a increasing saw wave
	if (m_samples != NOISEWAVE)
		delete m_samples;
	m_samples = new float[NOMINAL_WAVE_SIZE];
	m_numSamples = NOMINAL_WAVE_SIZE;
	for (int i = 0; i < NOMINAL_WAVE_SIZE; i++)
		{
		float a = 2.0f * ((float)i / NOMINAL_WAVE_SIZE) - 1.0f;
		m_samples[i] = a;
		}
}

/// @param[in] duty		Waveform duty cycle (0.0 to 1.0) (default = 0.5)
void CWaveform::MakeSaw2Wave(float duty)
{
	// Make a decreasing saw wave
	if (m_samples != NOISEWAVE)
		delete m_samples;
	m_samples = new float[NOMINAL_WAVE_SIZE];
	m_numSamples = NOMINAL_WAVE_SIZE;
	for (int i = 0; i < NOMINAL_WAVE_SIZE; i++)
		{
		float a = 1.0f - (2.0f * ((float)i / NOMINAL_WAVE_SIZE));
		m_samples[i] = a;
		}
}

/// @param[in] duty		Waveform duty cycle (0.0 to 1.0) (default = 0.5)
void CWaveform::MakeTriangleWave(float duty)
{
	// Make a triangle wave
	if (m_samples != NOISEWAVE)
		delete m_samples;
	m_samples = new float[NOMINAL_WAVE_SIZE];
	m_numSamples = NOMINAL_WAVE_SIZE;
	int quarterLength = NOMINAL_WAVE_SIZE / 4;
	for (int i = 0; i < quarterLength; i++)
		{
		m_samples[i] = (float)i / quarterLength;
		m_samples[i + quarterLength] = 1.0f - (float)i / quarterLength;
		m_samples[i + quarterLength * 2] = 0.0f - (float)i / quarterLength;
		m_samples[i + quarterLength * 3] = -1.0f + (float)i / quarterLength;
		}
}

void CWaveform::MakeNoiseWave()
{
	if (m_samples != NOISEWAVE)
		delete m_samples;

	// Create the one and only noise wave if it has not been set up
	if (0.0f == NOISEWAVE[0])
		{
		printf("Creating noise waveform...\n");

		unsigned int seed = 5323;
		for (int i = 0; i < NOISE_WAVE_SIZE; i++)
			{
			// Large constants and overflow make next number in sequence
			// very unpredictable (ie: "random")
			seed = (8253729 * seed + 2396403);
			int r = seed % 32767;
			NOISEWAVE[i] = (float)(r - 16384) / 16384.0f;
			}
			
		// TODO : Pass LPF over the noise to make it pink
		}
	
	// ...and set our waveform to point to it
	m_samples = NOISEWAVE;
	m_numSamples = NOISE_WAVE_SIZE;
}

/// Set custom waveform data
void CWaveform::SetCustomWaveData(float *data, unsigned int numSamples)
{
	if (data && numSamples > 20 && numSamples <= 1000000)
		{
		m_numSamples = numSamples;
		if (m_samples != NOISEWAVE)
			delete m_samples;
		m_samples = new float[numSamples];
		for (unsigned int i = 0; i < numSamples; i++)
			m_samples[i] = data[i];
		}
	else
		{
		// fallback - avoid "empty" wave
		MakeSquareWave();
		}
}

// static!
const char *CWaveform::GetWaveformName(WAVETYPE type)
{
	switch (type)
		{
		case WT_NONE :							// "off"
			return "NONE";
			break;
		case WT_SQUARE :
			return "SQUARE";
			break;
		case WT_SAW1 :
			return "SAW1";
			break;
		case WT_SAW2 :
			return "SAW2";
			break;
		case WT_TRIANGLE :
			return "TRIANGLE";
			break;
		case WT_SINE :
			return "SINE";
			break;
		case WT_NOISE :
			return "NOISE";
			break;
		case WT_CUSTOM :
			return "CUSTOM";
			break;
		case WT_MAX :			// so compiler doesn't complain
			break;
		}
		
	return "bad!";
}

