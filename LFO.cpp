#include "LFO.h"
#include "Messages.h"

CLFO::CLFO()
	: m_waveType(WT_NONE),					// disable
		m_freq(1.0f),
		m_volumeEnv(0.0f, 0.0f, 0.0f, 0.0f),
		m_pitchEnv(0.0f, 0.0f, 0.0f, 0.0f),
		m_filterEnv(0.0f, 0.0f, 0.0f, 0.0f),
		m_PWMEnv(0.0f, 0.0f, 0.0f, 0.0f)
{
}

CLFO::~CLFO()
{
}

/// Pack the LFO parameter data into a message buffer
int CLFO::PackParams(char *buffer)
{
	buffer[0] = (char)m_waveType;
	buffer[1] = PackScaledValue(m_freq / LFO_MAX_FREQ, 0x7F);
	m_volumeEnv.PackParams(buffer + 2);
	m_pitchEnv.PackParams(buffer + 6);
	m_filterEnv.PackParams(buffer + 10);
	m_PWMEnv.PackParams(buffer + 14);
	
	return PADDR_MOD1 - PADDR_LFO;	// number of bytes added to the buffer
}

/// Unpack the LFO param data from a message and update
/// the LFO
void CLFO::UnpackParams(char *buffer)
{
	m_waveType = (WAVETYPE)buffer[0];
	m_freq = UnpackValue(buffer[1], 0x7F) * LFO_MAX_FREQ;
	m_volumeEnv.UnpackParams(buffer + 2);
	m_pitchEnv.UnpackParams(buffer + 6);
	m_filterEnv.UnpackParams(buffer + 10);
	m_PWMEnv.UnpackParams(buffer + 14);
}
