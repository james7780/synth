/// Class to handle one voice of a polyphonic system
#pragma once
#include "Patch.h"

class CVoice
{
public:
	CVoice();
	~CVoice();

	// Restart this voice
	//void NoteOn(const CPatch *patch, char note, float v);
	void NoteOn(const COscillator *osc, char note, float v);

	// note up this voice
	void NoteOff();

	// Stop this voice
	void Stop();

	// Update this voice's state for n ms
	void Update(float millis);

	// Get current volume/pitch/filter envelope level for this voice
	float GetEnvelopeLevel(ENVTYPE which);
	// Get current LFO volume/pitch/filter envelope level for this voice
	float GetLFOEnvelopeLevel(ENVTYPE which);

	// The patch this voice is using
	//const CPatch *m_patch;
	// The oscillator this voice is using
	const COscillator *m_osc;
	// Internal copy of the waveform being used
	CWaveform m_waveform;
	// Current waveform playback position in this voice
	int m_wavePos;
	// Playback frequency
	float m_freq;
	// Voice volume
	float m_volume;						// 0.0 to 1.0
	// Envelope time since note on
	float m_elapsedTime;
	// This voice has released (note off)
	bool m_released;
	// The envelope level when we released the note
	float m_releasedLevel;
	
	// The base note being played by this voice
	char m_note;

};

