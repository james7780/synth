#include "WaveSynth.h"
#include "Messages.h"
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <algorithm>

#define ZEROVOLTOLERANCE 0.01

CWaveSynth::CWaveSynth()
	: m_currentPatchIndex(0)
{
}

CWaveSynth::~CWaveSynth()
{
}

// http://subsynth.sourceforge.net/midinote2freq.html
// STATIC
float CWaveSynth::CalcMidiNoteFreqency(int midiNoteNumber)
{
	//const float a = 440.0f; // a is 440 hz...
	float freq = 440.0f * pow(2.0f, (float)(midiNoteNumber - 69) / 12);
	return freq;
}

/// Load patch data from disk
void CWaveSynth::LoadPatches()
{
	FILE *pf = fopen("patches.dat", "r");
	if (pf)
		{
		// Patch to read into
		CPatch *patch = &m_patches[0];
		while (!feof(pf))
			{
			// read and parse line
			char buffer[100];
			if (!fgets(buffer, 100, pf))
				break;
				
			// split line into param and value
			char *pValue = strchr(buffer, '=');
			if (pValue)
				{
				*pValue = 0;
				pValue++;
				char *pParam = buffer;
				// Remove '\n' off end of the value
				pValue[strlen(pValue) - 1] = 0;

				// parse
				if (0 == strcmp(pParam, "patch"))
					{
					int n = std::min(atoi(pValue), NUM_PATCHES - 1);
					patch = &m_patches[n];
					}
				else if (0 == strcmp(pParam, "name"))
					patch->SetName(pValue);
				else if (0 == strcmp(pParam, "mixlevel"))
					patch->SetMixLevel(atof(pValue));
				else if (0 == strcmp(pParam, "lfo.wavetype"))
					patch->m_LFOWaveform = (WAVETYPE)atoi(pValue);
				else if (0 == strcmp(pParam, "lfo.freq"))
					patch->m_LFOFreq = atof(pValue);
				else if (0 == strcmp(pParam, "lfo.depth"))
					patch->m_LFODepth = atof(pValue);
					
				else if (0 == strcmp(pParam, "osc1.wavetype"))
					patch->m_osc1.m_waveType = (WAVETYPE)atoi(pValue);
				else if (0 == strcmp(pParam, "osc1.duty"))
					patch->m_osc1.m_duty = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.detune"))
					patch->m_osc1.m_detune = atof(pValue);

				else if (0 == strcmp(pParam, "osc1.volenvDL"))
					patch->m_osc1.m_envelope[ET_VOLUME].m_delay = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.volenvA"))
					patch->m_osc1.m_envelope[ET_VOLUME].m_attack = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.volenvP"))
					patch->m_osc1.m_envelope[ET_VOLUME].m_peak = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.volenvD"))
					patch->m_osc1.m_envelope[ET_VOLUME].m_decay = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.volenvS"))
					patch->m_osc1.m_envelope[ET_VOLUME].m_sustain = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.volenvR"))
					patch->m_osc1.m_envelope[ET_VOLUME].m_release = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.pitchenvDL"))
					patch->m_osc1.m_envelope[ET_PITCH].m_delay = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.pitchenvA"))
					patch->m_osc1.m_envelope[ET_PITCH].m_attack = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.pitchenvP"))
					patch->m_osc1.m_envelope[ET_PITCH].m_peak = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.pitchenvD"))
					patch->m_osc1.m_envelope[ET_PITCH].m_decay = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.pitchenvS"))
					patch->m_osc1.m_envelope[ET_PITCH].m_sustain = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.pitchenvR"))
					patch->m_osc1.m_envelope[ET_PITCH].m_release = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.filterenvDL"))
					patch->m_osc1.m_envelope[ET_FILTER].m_delay = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.filterenvA"))
					patch->m_osc1.m_envelope[ET_FILTER].m_attack = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.filterenvP"))
					patch->m_osc1.m_envelope[ET_FILTER].m_peak = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.filterenvD"))
					patch->m_osc1.m_envelope[ET_FILTER].m_decay = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.filterenvS"))
					patch->m_osc1.m_envelope[ET_FILTER].m_sustain = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.filterenvR"))
					patch->m_osc1.m_envelope[ET_FILTER].m_release = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOVolenvDL"))
					patch->m_osc1.m_LFOEnvelope[ET_VOLUME].m_delay = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOVolenvA"))
					patch->m_osc1.m_LFOEnvelope[ET_VOLUME].m_attack = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOVolenvP"))
					patch->m_osc1.m_LFOEnvelope[ET_VOLUME].m_peak = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOVolenvD"))
					patch->m_osc1.m_LFOEnvelope[ET_VOLUME].m_decay = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOVolenvS"))
					patch->m_osc1.m_LFOEnvelope[ET_VOLUME].m_sustain = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOVolenvR"))
					patch->m_osc1.m_LFOEnvelope[ET_VOLUME].m_release = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOPitchenvDL"))
					patch->m_osc1.m_LFOEnvelope[ET_PITCH].m_delay = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOPitchenvA"))
					patch->m_osc1.m_LFOEnvelope[ET_PITCH].m_attack = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOPitchenvP"))
					patch->m_osc1.m_LFOEnvelope[ET_PITCH].m_peak = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOPitchenvD"))
					patch->m_osc1.m_LFOEnvelope[ET_PITCH].m_decay = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOPitchenvS"))
					patch->m_osc1.m_LFOEnvelope[ET_PITCH].m_sustain = atof(pValue);
				else if (0 == strcmp(pParam, "osc1.LFOPitchenvR"))
					patch->m_osc1.m_LFOEnvelope[ET_PITCH].m_release = atof(pValue);
// TODO : More LFO envelopes
				else if (0 == strcmp(pParam, "osc2.wavetype"))
					patch->m_osc2.m_waveType = (WAVETYPE)atoi(pValue);
				else if (0 == strcmp(pParam, "osc2.duty"))
					patch->m_osc2.m_duty = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.detune"))
					patch->m_osc2.m_detune = atof(pValue);
					
				else if (0 == strcmp(pParam, "osc2.volenvDL"))
					patch->m_osc2.m_envelope[ET_VOLUME].m_delay = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.volenvA"))
					patch->m_osc2.m_envelope[ET_VOLUME].m_attack = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.volenvP"))
					patch->m_osc2.m_envelope[ET_VOLUME].m_peak = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.volenvD"))
					patch->m_osc2.m_envelope[ET_VOLUME].m_decay = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.volenvS"))
					patch->m_osc2.m_envelope[ET_VOLUME].m_sustain = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.volenvR"))
					patch->m_osc2.m_envelope[ET_VOLUME].m_release = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.pitchenvDL"))
					patch->m_osc2.m_envelope[ET_PITCH].m_delay = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.pitchenvA"))
					patch->m_osc2.m_envelope[ET_PITCH].m_attack = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.pitchenvP"))
					patch->m_osc2.m_envelope[ET_PITCH].m_peak = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.pitchenvD"))
					patch->m_osc2.m_envelope[ET_PITCH].m_decay = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.pitchenvS"))
					patch->m_osc2.m_envelope[ET_PITCH].m_sustain = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.pitchenvR"))
					patch->m_osc2.m_envelope[ET_PITCH].m_release = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.filterenvDL"))
					patch->m_osc2.m_envelope[ET_FILTER].m_delay = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.filterenvA"))
					patch->m_osc2.m_envelope[ET_FILTER].m_attack = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.filterenvP"))
					patch->m_osc2.m_envelope[ET_FILTER].m_peak = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.filterenvD"))
					patch->m_osc2.m_envelope[ET_FILTER].m_decay = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.filterenvS"))
					patch->m_osc2.m_envelope[ET_FILTER].m_sustain = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.filterenvR"))
					patch->m_osc2.m_envelope[ET_FILTER].m_release = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOVolenvDL"))
					patch->m_osc2.m_LFOEnvelope[ET_VOLUME].m_delay = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOVolenvA"))
					patch->m_osc2.m_LFOEnvelope[ET_VOLUME].m_attack = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOVolenvP"))
					patch->m_osc2.m_LFOEnvelope[ET_VOLUME].m_peak = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOVolenvD"))
					patch->m_osc2.m_LFOEnvelope[ET_VOLUME].m_decay = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOVolenvS"))
					patch->m_osc2.m_LFOEnvelope[ET_VOLUME].m_sustain = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOVolenvR"))
					patch->m_osc2.m_LFOEnvelope[ET_VOLUME].m_release = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOPitchenvDL"))
					patch->m_osc2.m_LFOEnvelope[ET_PITCH].m_delay = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOPitchenvA"))
					patch->m_osc2.m_LFOEnvelope[ET_PITCH].m_attack = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOPitchenvP"))
					patch->m_osc2.m_LFOEnvelope[ET_PITCH].m_peak = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOPitchenvD"))
					patch->m_osc2.m_LFOEnvelope[ET_PITCH].m_decay = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOPitchenvS"))
					patch->m_osc2.m_LFOEnvelope[ET_PITCH].m_sustain = atof(pValue);
				else if (0 == strcmp(pParam, "osc2.LFOPitchenvR"))
					patch->m_osc2.m_LFOEnvelope[ET_PITCH].m_release = atof(pValue);
				
				}	// end if pValue
			}	// wend
		
		fclose(pf);
		}
}

void CWaveSynth::SavePatches()
{
	FILE *pf = fopen("patches.dat", "w");
	if (pf)
		{
		for (int i = 0; i < NUM_PATCHES; i++)
			{
			CPatch *patch = &m_patches[i];
			fprintf(pf, "patch=%d\n", i);
			fprintf(pf, "name=%s\n", patch->GetName());
			fprintf(pf, "mixlevel=%.2f\n", patch->GetMixLevel());

			fprintf(pf, "lfo.wavetype=%df\n", patch->m_LFOWaveform);
			fprintf(pf, "lfo.freq=%.2f\n", patch->m_LFOFreq);
			fprintf(pf, "lfo.depth=%.2f\n", patch->m_LFODepth);
			
			COscillator *osc1 = &patch->m_osc1;
			fprintf(pf, "osc1.wavetype=%d\n", osc1->m_waveType);
			fprintf(pf, "osc1.duty=%.2f\n", osc1->m_duty);
			fprintf(pf, "osc1.detune=%.2f\n", osc1->m_detune);
			CEnvelope &env = osc1->m_envelope[ET_VOLUME];
			fprintf(pf, "osc1.volenvDL=%d\n", (int)env.m_delay);
			fprintf(pf, "osc1.volenvA=%d\n", (int)env.m_attack);
			fprintf(pf, "osc1.volenvP=%d\n", (int)env.m_peak);
			fprintf(pf, "osc1.volenvD=%d\n", (int)env.m_decay);
			fprintf(pf, "osc1.volenvS=%.2f\n", env.m_sustain);
			fprintf(pf, "osc1.volenvR=%d\n", (int)env.m_release);
			env = osc1->m_envelope[ET_PITCH];
			fprintf(pf, "osc1.pitchenvDL=%d\n", (int)env.m_delay);
			fprintf(pf, "osc1.pitchenvA=%d\n", (int)env.m_attack);
			fprintf(pf, "osc1.pitchenvP=%d\n", (int)env.m_peak);
			fprintf(pf, "osc1.pitchenvD=%d\n", (int)env.m_decay);
			fprintf(pf, "osc1.pitchenvS=%.2f\n", env.m_sustain);
			fprintf(pf, "osc1.pitchenvR=%d\n", (int)env.m_release);
			env = osc1->m_envelope[ET_FILTER];
			fprintf(pf, "osc1.filterenvDL=%d\n", (int)env.m_delay);
			fprintf(pf, "osc1.filterenvA=%d\n", (int)env.m_attack);
			fprintf(pf, "osc1.filterenvP=%d\n", (int)env.m_peak);
			fprintf(pf, "osc1.filterenvD=%d\n", (int)env.m_decay);
			fprintf(pf, "osc1.filterenvS=%.2f\n", env.m_sustain);
			fprintf(pf, "osc1.filterenvR=%d\n", (int)env.m_release);
			env = osc1->m_LFOEnvelope[ET_VOLUME];
			fprintf(pf, "osc1.LFOVolenvDL=%d\n", (int)env.m_delay);
			fprintf(pf, "osc1.LFOVolenvA=%d\n", (int)env.m_attack);
			fprintf(pf, "osc1.LFOVolenvP=%d\n", (int)env.m_peak);
			fprintf(pf, "osc1.LFOVolenvD=%d\n", (int)env.m_decay);
			fprintf(pf, "osc1.LFOVolenvS=%.2f\n", env.m_sustain);
			fprintf(pf, "osc1.LFOVolenvR=%d\n", (int)env.m_release);
			env = osc1->m_LFOEnvelope[ET_PITCH];
			fprintf(pf, "osc1.LFOPitchenvDL=%d\n", (int)env.m_delay);
			fprintf(pf, "osc1.LFOPitchenvA=%d\n", (int)env.m_attack);
			fprintf(pf, "osc1.LFOPitchenvP=%d\n", (int)env.m_peak);
			fprintf(pf, "osc1.LFOPitchenvD=%d\n", (int)env.m_decay);
			fprintf(pf, "osc1.LFOPitchenvS=%.2f\n", env.m_sustain);
			fprintf(pf, "osc1.LFOPitchenvR=%d\n", (int)env.m_release);

			COscillator *osc2 = &patch->m_osc2;
			fprintf(pf, "osc2.wavetype=%d\n", osc2->m_waveType);
			fprintf(pf, "osc2.duty=%.2f\n", osc2->m_duty);
			fprintf(pf, "osc2.detune=%.2f\n", osc2->m_detune);
			env = osc2->m_envelope[ET_VOLUME];
			fprintf(pf, "osc2.volenvDL=%d\n", (int)env.m_delay);
			fprintf(pf, "osc2.volenvA=%d\n", (int)env.m_attack);
			fprintf(pf, "osc2.volenvP=%d\n", (int)env.m_peak);
			fprintf(pf, "osc2.volenvD=%d\n", (int)env.m_decay);
			fprintf(pf, "osc2.volenvS=%.2f\n", env.m_sustain);
			fprintf(pf, "osc2.volenvR=%d\n", (int)env.m_release);
			env = osc2->m_envelope[ET_PITCH];
			fprintf(pf, "osc2.pitchenvDL=%d\n", (int)env.m_delay);
			fprintf(pf, "osc2.pitchenvA=%d\n", (int)env.m_attack);
			fprintf(pf, "osc2.pitchenvP=%d\n", (int)env.m_peak);
			fprintf(pf, "osc2.pitchenvD=%d\n", (int)env.m_decay);
			fprintf(pf, "osc2.pitchenvS=%.2f\n", env.m_sustain);
			fprintf(pf, "osc2.pitchenvR=%d\n", (int)env.m_release);
			env = osc2->m_envelope[ET_FILTER];
			fprintf(pf, "osc2.filterenvDL=%d\n", (int)env.m_delay);
			fprintf(pf, "osc2.filterenvA=%d\n", (int)env.m_attack);
			fprintf(pf, "osc2.filterenvP=%d\n", (int)env.m_peak);
			fprintf(pf, "osc2.filterenvD=%d\n", (int)env.m_decay);
			fprintf(pf, "osc2.filterenvS=%.2f\n", env.m_sustain);
			fprintf(pf, "osc2.filterenvR=%d\n", (int)env.m_release);
			env = osc2->m_LFOEnvelope[ET_VOLUME];
			fprintf(pf, "osc2.LFOVolenvDL=%d\n", (int)env.m_delay);
			fprintf(pf, "osc2.LFOVolenvA=%d\n", (int)env.m_attack);
			fprintf(pf, "osc2.LFOVolenvP=%d\n", (int)env.m_peak);
			fprintf(pf, "osc2.LFOVolenvD=%d\n", (int)env.m_decay);
			fprintf(pf, "osc2.LFOVolenvS=%.2f\n", env.m_sustain);
			fprintf(pf, "osc2.LFOVolenvR=%d\n", (int)env.m_release);
			env = osc2->m_LFOEnvelope[ET_PITCH];
			fprintf(pf, "osc2.LFOPitchenvDL=%d\n", (int)env.m_delay);
			fprintf(pf, "osc2.LFOPitchenvA=%d\n", (int)env.m_attack);
			fprintf(pf, "osc2.LFOPitchenvP=%d\n", (int)env.m_peak);
			fprintf(pf, "osc2.LFOPitchenvD=%d\n", (int)env.m_decay);
			fprintf(pf, "osc2.LFOPitchenvS=%.2f\n", env.m_sustain);
			fprintf(pf, "osc2.LFOPitchenvR=%d\n", (int)env.m_release);
				
			}
		
		fclose(pf);
		}
}

/// Select a patch from the bank of patches
void CWaveSynth::SelectPatch(int patchNum)
{
	if (patchNum >= 0 && patchNum < NUM_PATCHES)
		{
		m_workPatch.CopyFrom(m_patches[patchNum]);
		m_currentPatchIndex = patchNum;
		printf("WaveSynth: patch %d selected\n", patchNum);
		}
}

/// Store current working patch into the selected patch slot
void CWaveSynth::StoreWorkPatch(int patchNum)
{
	if (patchNum >= 0 && patchNum < NUM_PATCHES)
		{
		m_patches[patchNum].CopyFrom(m_workPatch);
		printf("WaveSynth: Stored work to patch #%d\n", patchNum);
		}
}

/// Process messages for the synth
/// @param buffer			Message buffer
/// @param length			Length of the message
void CWaveSynth::ProcessMessage(char *buffer, int length)
{
	if (length < 1)
		return;

	//printf("msg: %2X, %2X, len = %d\n", buffer[0], buffer[1], length);

	// All messages are MIDI messages
	char command = buffer[0];
	switch (command & 0xF0)
		{
		case 0x80 :			// Note Off
			{
			char note = buffer[1];
			NoteOff(note);
			}
			break;
		case 0x90 :			// Note On
			{
			char note = buffer[1];
			float vol = UnpackValue(buffer[2], 0x7F);
			NoteOn(-1, note, vol);
			}
			break;
		case 0xB0 :			// Controller change
			{
			char cc = buffer[1];
			if (1 == cc)			// MOD wheel
				{
				// LPFilter max = 2khz
				float freq = 2000.0f + 2000.0f * ((float)buffer[2] / 0x7F); 
				m_mixer.m_LPFilter.SetCutoffFreq(freq);
				}
			else if (74 == cc)		// Filter cutouff
				{
				// LPFilter max = 2khz
				float freq = 2000.0f * ((float)buffer[2] / 0x7F); 
				m_mixer.m_LPFilter.SetCutoffFreq(freq);
				}
			else if (91 == cc)		// Reverb Send
				{
				// LPFilter max = 2khz
				float sendValue = ((float)buffer[2] / 0x7F); 
				printf("WaveSynth: reverb level = %.2f\n", sendValue);
				m_mixer.m_reverb.m_mixLevel = sendValue;	
				}
			}
			break;
		case 0xC0 :			// Patch change
			{
			char patchIndex = buffer[1];
			SelectPatch(patchIndex);
			}
			break;
		case 0xE0 :			// Pitch bend
			{
/*
			char bend = buffer[1];
			// TEMP - Use to test cutoff value
			// LPFilter max = 2khz
			float freq = 300.0f + 8000.0f * ((float)bend / 0x7F); 
			m_mixer.m_LPFilter.SetCutoffFreq(freq);
*/
			// PB = 2 x 7-bit values = 0 to 16383 (-8191 to + 8191) 
			int bend = buffer[1] + 128 * buffer[2];
			// Update mixer's "bend ratio"
			if (bend > 8200)
				m_mixer.m_targetBendRatio = 1.0f + 0.059463f * ((float)(bend - 8192) / 8192);
			else if (bend < 8184)
				m_mixer.m_targetBendRatio = 0.943874f + 0.056125f * ((float)bend / 8192);
			else
				m_mixer.m_targetBendRatio = 1.0f;
			printf("bend = %d, ratio = %.3f\n", bend, m_mixer.m_targetBendRatio);
			}
			break;
		case 0XF0 :			// System / sysex
			{
			// Handled higher up in SynthEngine
			}
			break;
		}	// end switch
}

/// Trigger a new note on using the specified patch settings
/// @param patchIndex		The patch to use (or -1 for work patch)
/// @param note				MIDI note number
/// @param volume			The mix volume of the note
void CWaveSynth::NoteOn(short patchIndex, char note, float volume)
{
	// Handle NoteOn(vol = 0) ( = NoteOff)
	if (volume < ZEROVOLTOLERANCE)
		{
		NoteOff(note);
		return;
		}
		
	const CPatch *patch = &m_workPatch;
	if (patchIndex >= 0 && patchIndex < NUM_PATCHES)
		{
		patch = &m_patches[patchIndex];
		}
		
	// Get the first free voice
	// TODO : Replace oldest voice if none free
	//CVoice *voice1 = mixer.GetVoiceFromNote(note);
	if (patch->m_osc1.m_waveType > WT_NONE)
		{
		CVoice *voice1 = m_mixer.GetFreeVoice();
		if (voice1)
			{
			// Note: Voice must know which Oscillator to "use"
			//voice->NoteOn(&patches[patchIndex], note, volume);
			voice1->NoteOn(&patch->m_osc1, note, volume * m_workPatch.GetMixLevel());
			//printf("NoteOn: note number = %d, freq = %.2f\n", note, voice1->m_freq);
			}
		}

	if (patch->m_osc2.m_waveType > WT_NONE)
		{
		CVoice *voice2 = m_mixer.GetFreeVoice();
		if (voice2)
			{
			// Note: Voice must know which Oscillator to "use"
			//voice->NoteOn(&patches[patchIndex], note, volume);
			voice2->NoteOn(&patch->m_osc2, note, volume * m_workPatch.GetMixLevel());
			//printf("NoteOn: note number = %d, freq = %.2f\n", note, voice2->m_freq);
			}
		}

}

void CWaveSynth::NoteOff(char note)
{
	// Find the voice playing this note, and set it to release mode
	// Note : We will have to run thru all the voices in the mixer since
	//        it is possible that 5 voices could be playing the same note.
	for (int n = 0; n < NUM_VOICES; n++)
		{
		if (m_mixer.m_voice[n].m_note == note)
			{
			m_mixer.m_voice[n].NoteOff();
			}
		}
}
/// Update all voices in this wave synth
/// @param millis				Number of millisecs since last update
void CWaveSynth::Update(float millis)
{
		// Update all active voices
	for (int n = 0; n < NUM_VOICES; n++)
		{
		m_mixer.m_voice[n].Update(millis);
		}

}

/// Get the number of voices currently playing
int CWaveSynth::GetActiveVoiceCount() const
{
	int count = 0;
	for (int n = 0; n < NUM_VOICES; n++)
		{
		if (m_mixer.m_voice[n].m_wavePos > -1)
			count++;
		}

	return count;
}
