/// Class representing a reverb effect
#pragma once

// Roughly 10 secs reverb buffers
#define REVERB_BUFFER_LENGTH	480000

/// Base reverb class
class CReverb
{
public:
	CReverb(float delay, float decay);

	/// Set delay
	virtual void SetDelay(float delay);
	/// Set decay
	virtual void SetDecay(float decay);
	/// Apply this reverb to an output buffer
	void Apply(float *buffer, int length, int sampleRate);

	float m_mixLevel;				// reverb "send" level

protected:
	float m_delay;					// ms
	float m_decay;					// 0.0 to 1.0
	float m_rvbBuffer[REVERB_BUFFER_LENGTH];	// circular buffer
	int m_pos;						// current buffer position
};
