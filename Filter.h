/// Class representing a filter
#pragma once

/// Base LP filter class
class CLPFilter
{
public:
	CLPFilter(float sampleRate, float cutOff);

	/// Set filter cutoff freqency
	virtual void SetCutoffFreq(float freq) = 0;
	/// Set filter resonance
	virtual void SetResonance(float res) = 0;
	/// Apply LP Filter to the output buffer
	virtual void Apply(float *buffer, int length) = 0;

protected:
	float m_sampleRate;
	//float m_cutoff;
	float m_resonance;
};

/// Simple 1-pole LP filter class (-6dB per octave)
class C1PoleLPFilter : public CLPFilter
{
public:
	C1PoleLPFilter(float sampleRate, float cutOff);

	/// Set filter cutoff freqency
	void SetCutoffFreq(float freq);
	/// Set filter resonance
	void SetResonance(float res);
	/// Apply LP Filter to the output buffer
	void Apply(float *buffer, int length);

private:	
	float m_a;
	float m_b;
	float m_prevValue;
};

/// Moog 4-pole LP filter class (-6dB per octave)
class CMoogLPFilter : public CLPFilter
{
public:
	CMoogLPFilter(float sampleRate, float cutOff);

	/// Set filter cutoff freqency
	void SetCutoffFreq(float freq);
	/// Set filter resonance
	void SetResonance(float res);
	/// Apply LP Filter to the output buffer
	void Apply(float *buffer, int length);
	
private:	
	float m_k;
	float m_p;
	float m_r;
	float m_prevX;
	float m_prevY1, m_prevY2, m_prevY3;
	float m_Y4;
};
