/// One "patch" or "preset" in the synthesizer
#pragma once
//#include "Waveform.h"
//#include "Envelope.h"
#include "Oscillator.h"
#include "Modulator.h"
#include "LFO.h"

#define PATCH_NAME_LEN	20

class CPatch
{
public:
	CPatch();
	~CPatch();

	void SetName(const char *name);
	const char *GetName() const;
	void SetMixLevel(float level);
	float GetMixLevel() const;

	void CopyFrom(const CPatch &rhs);
	/// Pack the patch parameter data into a message buffer
	int PackParams(char *buffer);
	/// Unpack the patch param data from a message and update the patch
	void UnpackParams(char *buffer, char address, char size);
	
	void Dump();

public:
	COscillator m_osc1;				// OSC1 settings
	COscillator m_osc2;				// OSC2 settings
	CModulator m_modulator[2];		// Pitch bend and mod wheel
	// LFO presets for this patch
	WAVETYPE m_LFOWaveform;
	float m_LFOFreq;
	float m_LFODepth;

private:	
	char m_name[PATCH_NAME_LEN];	// Name of patch
	float m_mixLevel;				// Patch mix level

};

