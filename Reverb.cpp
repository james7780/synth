/// Implementation of reverb for the synth
#include <stdio.h>
#include <math.h>
#include "Reverb.h"

CReverb::CReverb(float delay, float decay)
	:	m_mixLevel(1.0f),
		m_delay(delay),
		m_decay(decay),
		m_pos(0)
{
	// init reverb buffer
	for (int i = 0; i < REVERB_BUFFER_LENGTH; i++)
		m_rvbBuffer[i] = 0.0f;
}


/// Set the reverb delay	
void CReverb::SetDelay(float delay)
{
	m_delay = delay;
	// calc other params here is neccessary
}

/// Set reverb decay
void CReverb::SetDecay(float decay)
{
	m_decay = decay;
}

/// Apply this reverb to an output buffer
/// @param[in] buffer			The output buffer
/// @param[in] length			The number of samples in the buffer
/// @param[in] sampleRate		The output sample rate
void CReverb::Apply(float *buffer, int length, int sampleRate)
{
	// 1. Combine delayed reverb buffer (with decay) to output buffer
	// 2. Append output buffer to reverb buffer (overwrite)
	
	// Find buffer pos delay ms back in time
	int reverbPos = m_pos - (m_delay / 1000.0f) * sampleRate;
	if (reverbPos < 0)
		reverbPos += REVERB_BUFFER_LENGTH;
	
	// 1. Combine delayed reverb buffer (with decay) to output buffer
	for (int i = 0; i < length; i++)
		{
		float in = buffer[i];
		float out = m_mixLevel * m_rvbBuffer[reverbPos] + in;
		buffer[i] = out;
		reverbPos++;
		if (REVERB_BUFFER_LENGTH == reverbPos)
			reverbPos = 0;
			
		// 2. Append output sample to reverb buffer (overwrite)
		m_rvbBuffer[m_pos] = out * m_decay;
		m_pos++;
		if (REVERB_BUFFER_LENGTH == m_pos)
			m_pos = 0;
		}

}


