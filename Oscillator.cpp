#include <stdio.h>
#include "Oscillator.h"
#include "Messages.h"

COscillator::COscillator()
	: m_waveType(WT_SINE),
		m_duty(0.5f),
		m_detune(0.0f)
{
	// vol/pitch/filter envelopes
	m_envelope[ET_VOLUME].Set(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	m_envelope[ET_PITCH].Set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	m_envelope[ET_FILTER].Set(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	m_envelope[ET_PWM].Set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	m_envelope[ET_MOD].Set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	
	// LFO envelopes (temp - copied from Patch)
	m_LFOEnvelope[ET_VOLUME].Set(1000.0f, 2000.0f, 0.5f, 2000.0f, 0.2f, 2000.0f);
	m_LFOEnvelope[ET_PITCH].Set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	m_LFOEnvelope[ET_FILTER].Set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	m_LFOEnvelope[ET_PWM].Set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	m_LFOEnvelope[ET_MOD].Set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	
}

COscillator::~COscillator()
{
}

void COscillator::CopyFrom(const COscillator &rhs)
{
	m_waveType = rhs.m_waveType;
	m_duty = rhs.m_duty;
	m_detune = rhs.m_detune;
	for (int i = 0; i < ET_MAX; i++)
		{
		m_envelope[i].CopyFrom(rhs.m_envelope[i]);
		m_LFOEnvelope[i].CopyFrom(rhs.m_LFOEnvelope[i]);
		}
}

/// Pack the oscillator parameter data into a message buffer
int COscillator::PackParams(char *buffer)
{
	buffer[0] = (char)m_waveType;
	buffer[1] = PackScaledValue(m_duty, 0x7F);
	buffer[2] = PackScaledValue((m_detune + 1.0f) * 0.5f, 0x7F);		// shift range to +ve	
//printf("osc.detune packed value = %d\n", buffer[2]);
	buffer[3] = 0;		// padding / unused
	m_envelope[ET_VOLUME].PackParams(buffer + 4);
	m_envelope[ET_PITCH].PackParams(buffer + 10);
	m_envelope[ET_FILTER].PackParams(buffer + 16);
	m_envelope[ET_PWM].PackParams(buffer + 22);

	m_LFOEnvelope[ET_VOLUME].PackParams(buffer + 28);
	m_LFOEnvelope[ET_PITCH].PackParams(buffer + 34);
	m_LFOEnvelope[ET_FILTER].PackParams(buffer + 40);
	m_LFOEnvelope[ET_PWM].PackParams(buffer + 46);
	
	return PADDR_OSC2 - PADDR_OSC1;	// number of bytes added to the buffer
}

/// Unpack the oscillator param data from a message and update
/// the oscillator
void COscillator::UnpackParams(char *buffer)
{
	m_waveType = (WAVETYPE)buffer[0];
	m_duty = UnpackValue(buffer[1], 0x7F);
	m_detune = UnpackValue(buffer[2], 0x7F) * 2.0f - 1.0f;
//printf("osc.detune unpacked value = %.3f\n", m_detune);
	m_envelope[ET_VOLUME].UnpackParams(buffer + 4);
	m_envelope[ET_PITCH].UnpackParams(buffer + 10);
	m_envelope[ET_FILTER].UnpackParams(buffer + 16);
	m_envelope[ET_PWM].PackParams(buffer + 22);

	m_LFOEnvelope[ET_VOLUME].UnpackParams(buffer + 28);
	m_LFOEnvelope[ET_PITCH].UnpackParams(buffer + 34);
	m_LFOEnvelope[ET_FILTER].UnpackParams(buffer + 40);
	m_LFOEnvelope[ET_PWM].UnpackParams(buffer + 46);

}

