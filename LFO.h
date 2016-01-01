/// LFO for use by a patch/voice
#pragma once

#include "Waveform.h"
#include "Envelope.h"

#define LFO_MAX_FREQ	20.0f

class CLFO
{
public:
	CLFO();
	~CLFO();

	// These are LFO settings used by Patches and Voices
	WAVETYPE m_waveType;
	float m_freq;				// LFO freqency
	CEnvelope m_volumeEnv;		//	(sets depth from "sustain" level)
	CEnvelope m_pitchEnv;		// 	(sets depth from "sustain" level)  (vibrato)
	CEnvelope m_filterEnv;		
	CEnvelope m_PWMEnv;
	
	/// Pack the LFO parameter data into a message buffer
	int PackParams(char *buffer);
	
	/// Unpack the LFO param data from a message
	void UnpackParams(char *buffer);

};

