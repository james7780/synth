/// Implementation of filters for the synth
#include <stdio.h>
#include <math.h>
//#include "common.h"
#include "Filter.h"

/////////////////////////////////////////////////////////////////////
/// Base LP Filter class
/////////////////////////////////////////////////////////////////////
CLPFilter::CLPFilter(float sampleRate, float cutOff)
{
	m_sampleRate = sampleRate;
	m_resonance = 0.5f;
}


/////////////////////////////////////////////////////////////////////
/// Simple 1-pole filter class
/////////////////////////////////////////////////////////////////////

C1PoleLPFilter::C1PoleLPFilter(float sampleRate, float cutOff)
	:	CLPFilter(sampleRate, cutOff)
{
	m_prevValue = 0.0f;
	SetCutoffFreq(cutOff);
}

/// Set the cutoff freq for the filter		
void C1PoleLPFilter::SetCutoffFreq(float freq)
{
	float k = exp(-2.0f * M_PI * freq / m_sampleRate);
	m_a = 1.0 - k;
	m_b = -k;
	printf("LPFilter: cutoff = %.0f, k = %.2f\n", freq, k);
}

/// Set filter resonance
void C1PoleLPFilter::SetResonance(float res)
{
	// Do nothing - resonance not implemented
}

/// Apply this filter to an output buffer
void C1PoleLPFilter::Apply(float *buffer, int length)
{
	for (int i = 0; i < length; i++)
		{
		float out = m_a * buffer[i] - m_b * m_prevValue;
		m_prevValue = out;
		buffer[i] = out;
		}
}


/////////////////////////////////////////////////////////////////////
/// Moog 4-pole filter class
/////////////////////////////////////////////////////////////////////

CMoogLPFilter::CMoogLPFilter(float sampleRate, float cutOff)
	:	CLPFilter(sampleRate, cutOff)
{
	SetCutoffFreq(cutOff);		// sets up m_k, m_p etc
	
	m_prevX = m_Y4 = 0.0f;
	m_prevY1 = m_prevY2 = m_prevY3 = 0.0f;
}

/// Set the cutoff freq for the filter		
void CMoogLPFilter::SetCutoffFreq(float freq)
{
	float f = 2.0f * freq / m_sampleRate;
	m_k = 3.6f * f - 1.6f * f * f - 1.0f;		// empirical
	m_p = (m_k + 1.0f) * 0.5f;
//	float scale = exp((1.0f - m_p) * 1.386249f);
	float scale = exp(1.0f - m_p) * 1.386249f;
	m_r = m_resonance * scale;
	
	printf("MoogFilter: cutoff = %.0f, k = %.2f\n", freq, m_k);
}

/// Set filter resonance
void CMoogLPFilter::SetResonance(float res)
{
	// TODO
}

/// Apply this filter to an output buffer
void CMoogLPFilter::Apply(float *buffer, int length)
{
	float y1 = 0.0f;
	float y2 = 0.0f;
	float y3 = 0.0f;
	
	for (int i = 0; i < length; i++)
		{
		// Inverted feedback for corner peaking
		float x = buffer[i] - m_r * m_Y4;
		
		// 4 cascaded 1-pole filters (bilinear transform)
		//y1 = x * m_p + m_prevX * m_p - m_k * y1;	// m_prevY1 ?
		//y2 = y1 * m_p + m_prevY1 * m_p - m_k * y2;
		//y3 = y2 * m_p + m_prevY2 * m_p - m_k * y3;
		//m_Y4 = y3 * m_p + m_prevY3 * m_p - m_k * m_Y4;

		y1 = x * m_p + m_prevX * m_p - m_k * m_prevY1;
		y2 = y1 * m_p + m_prevY1 * m_p - m_k * m_prevY2;
		y3 = y2 * m_p + m_prevY2 * m_p - m_k * m_prevY3;
		m_Y4 = y3 * m_p + m_prevY3 * m_p - m_k * m_Y4;
		
		// Clipper band-limited sigmoid (!)
		// m_Y4 = m_Y4 - (m_Y4 ^ 3) / 6;
		
		buffer[i] = m_Y4;
		
		m_prevX = x;
		m_prevY1 = y1;
		m_prevY2 = y2;
		m_prevY3 = y3;
		}
}
