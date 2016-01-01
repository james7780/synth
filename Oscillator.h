/// Patch Oscillator settings/params
#pragma once
#include "Waveform.h"
#include "Envelope.h"

class COscillator
{
public:
	COscillator();
	~COscillator();

	// wave shape
	WAVETYPE m_waveType;
	// wave duty cycle (0.0 to 1.0)
	float m_duty;
	// detune (in cents?) (should this be replaced by pitch envelope?)
	float m_detune;
	// vol/pitch/filter envelopes
	CEnvelope m_envelope[ET_MAX];
	// LFO envelopes (temp - copied from patch)
	CEnvelope m_LFOEnvelope[ET_MAX];

	void CopyFrom(const COscillator &rhs);
	/// Pack the patch parameter data into a message buffer
	int PackParams(char *buffer);
	/// Unpack the oscillator param data from a message and update
	/// the oscillator
	void UnpackParams(char *buffer);
};

