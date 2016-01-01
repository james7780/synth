/// Represents the wave synth engine
//- Manage Patches
//	- Patch collection
//	- Save / load patches
//	- Copy patches
//- Manages active voices
//	- Assigns voices (NoteOn())
//	- Tracks which voices belong to which patch
//	- Updates voices
//		- Update voice vol/pitch/filter from envelopes
//		- Update voice vol/pitch filter from LFO
//		- Disables voice after release finished
//- Mixes voices to output buffer (using CMixer)
//- Handles transpose
//- Recieves and processes MIDI messages:
//	- MIDI note on/off
//	- MIDI control change
//		- Pitch bend and modulation
//		- Patch change
//		- Tweak knobs
#pragma once

#include "Patch.h"
#include "Mixer.h"
#include <math.h>

#define NUM_PATCHES	32

class CWaveSynth
{
public:
	CWaveSynth();
	~CWaveSynth();

	/// Process messages for the synth
	void ProcessMessage(char *buffer, int length);
	
	/// Trigger a new note using the specified patch (-1 = work patch)
	void NoteOn(short patchIndex, char note, float volume);
	void NoteOff(char note);
	/// Update envelopes of allactive voices
	void Update(float periodMS);
	///  
	int GetActiveVoiceCount() const;

	/// Load patch data from disk
	void LoadPatches();
	void SavePatches();

	void SelectPatch(int patchNum);
	void StoreWorkPatch(int patchNum);
	
	// http://subsynth.sourceforge.net/midinote2freq.html
	static float CalcMidiNoteFreqency(int midiNoteNumber);

	CMixer m_mixer;
	CPatch m_patches[NUM_PATCHES];
	CPatch m_workPatch;				// copy of the current patch
	
	unsigned short m_currentPatchIndex;
};

