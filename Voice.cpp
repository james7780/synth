/// Class to handle one voice of a polyphonic system

#include <stdio.h>
#include <stdlib.h>			// for rand()
#include "Voice.h"
#include "WaveSynth.h"

CVoice::CVoice()
	:	m_osc(NULL),
		m_waveform(WT_SQUARE),
		m_wavePos(-1),							// -1 = inactive
		m_freq(440),
		m_volume(0.75f),
		m_elapsedTime(0.0f),
		m_released(false), 
		m_note(0)
{
}

CVoice::~CVoice()
{
}

/// Restart this voice
/// @param patch		The patch to use
/// @param note			MIDI note number
/// @param v			Mix volume (0.0 to 1.0)
//void CVoice::NoteOn(const CPatch *patch, char note, float v)
void CVoice::NoteOn(const COscillator *osc, char note, float v)
{
	// Update patch used and copy patch waveform
	if (osc)
		{
		m_osc = osc;
		// update waveform if different from current
		if (m_waveform.m_type != m_osc->m_waveType)
			{
			m_waveform.SetWaveform(m_osc->m_waveType);
			}
		}
	m_freq = CWaveSynth::CalcMidiNoteFreqency(note);
	// apply detune
	float d = m_freq * m_osc->m_detune * 0.01;
	m_freq += d;
	m_volume = v;
	m_elapsedTime = 0.0f;
	m_released = false;
	m_note = note;
	//m_wavePos = 0;					// enables this voice
	m_wavePos = rand() % 16;
	
	//printf("Voice noteon: freq = %.3f\n", m_freq);
}

/// Note release for this voice
void CVoice::NoteOff()
{
	// Set this voice to "released" mode (if not already)
	if (!m_released)
		{
		m_elapsedTime = 0.0f;
		m_released = true;
		}
}

// Stop this voice
void CVoice::Stop()
{
	m_wavePos = -1;
	m_note = 0;
}

/// Update this voice's state for n ms
/// @param millis				Number of millisecs since last update
void CVoice::Update(float millis)
{
	// Only process this voice if it is active
	if (m_wavePos > -1 && m_osc)
		{
		// This is done in CMixer::FillBuffer()
		//elapsedTime += millis;

		// Do we need to switch this voice off?
		if (m_released && m_elapsedTime > m_osc->m_envelope[ET_VOLUME].m_release)
			{
			Stop();
			}
		}

}

/// Get current volume/pitch/filter envelop level for this voice
float CVoice::GetEnvelopeLevel(ENVTYPE which)
{
	if (m_osc)
		return m_osc->m_envelope[which].GetLevel(m_elapsedTime, m_released);
	else
		return 0.0f;
}

/// Get current LFO volume/pitch/filter envelop level for this voice
float CVoice::GetLFOEnvelopeLevel(ENVTYPE which)
{
	if (m_osc)
		return m_osc->m_LFOEnvelope[which].GetLevel(m_elapsedTime, m_released);
	else
		return 0.0f;
}
