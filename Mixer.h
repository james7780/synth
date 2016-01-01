/// Class to mix all voices to the output buffer
#pragma once
#include "Voice.h"
#include "Filter.h"
#include "Reverb.h"

#define NUM_VOICES	32
#define MIXBUFFER_LENGTH	4096

#define CHROMATIC_RATIO		1.059463f
#define INV_CHR_RATIO		0.943874f

#define LFO_MAX_FREQ	20.0f

class CMixer
{
public:
	CMixer();
	~CMixer();

	/// Master mixing callback
	void FillBuffer(short* buffer, int numSamples, int sampleRate);

	/// Find the first free voice
	CVoice *GetFreeVoice();
	/// Get the voice which is playing the specified note
	CVoice *GetVoiceFromNote(char note);
	
	CVoice m_voice[NUM_VOICES];
	float m_masterVolume;						// 0.0 to 1.0
	float m_mixBuffer[MIXBUFFER_LENGTH];
	CWaveform m_LFOWaveform;
	float m_LFOFreq;
	float m_LFODepth;
	// The output filter
	//C1PoleLPFilter m_LPFilter;
	CMoogLPFilter m_LPFilter;
	// Output reverb effect
	CReverb m_reverb;
	
	float m_bendRatio;				// INV_CHR_RATIO to +CHROMATIC_RATIO
	float m_targetBendRatio;		// for smoothing of bending
};

