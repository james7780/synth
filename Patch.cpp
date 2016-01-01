/// One "patch" or "preset" in the synthesizer

#include "Patch.h"
#include "Messages.h"
#include "string.h"
#include <stdio.h>

CPatch::CPatch()
{
	strcpy(m_name, "\0");
	m_mixLevel = 1.0f;
	m_LFOWaveform = WT_SINE;
	m_LFOFreq = 1.0f;
	m_LFODepth = 1.0f;
	
	// Set up mod[0] as pitch bend, mod[1] as mod wheel (filter)
	m_modulator[0].m_modTarget = MT_PITCH;
	m_modulator[0].m_modRange = 24;
	m_modulator[1].m_modTarget = MT_FILTER;
	m_modulator[1].m_modRange = 127;
	
	// All other paramters set up by COscillator, CModulator, CLFO contructors
}

CPatch::~CPatch()
{
}

void CPatch::SetName(const char *name)
{
	strncpy(m_name, name, PATCH_NAME_LEN);
}

const char *CPatch::GetName() const
{
	return (const char*)m_name;
}

void CPatch::SetMixLevel(float level)
{
	m_mixLevel = level;
	if (m_mixLevel < 0.0f)
		m_mixLevel = 0.0f;
	else if (m_mixLevel > 1.0f)
		m_mixLevel = 1.0f;
}

float CPatch::GetMixLevel() const
{
	return m_mixLevel;
}

/// Copy patch settings from anotehr patch
void CPatch::CopyFrom(const CPatch &rhs)
{
	strcpy(m_name, rhs.m_name);
	m_mixLevel = rhs.m_mixLevel;
	m_LFOWaveform = rhs.m_LFOWaveform;
	m_LFOFreq = rhs.m_LFOFreq;
	m_LFODepth = rhs.m_LFODepth;
	m_modulator[0].CopyFrom(rhs.m_modulator[0]);
	m_modulator[1].CopyFrom(rhs.m_modulator[1]);
	m_osc1.CopyFrom(rhs.m_osc1);
	m_osc2.CopyFrom(rhs.m_osc2);
}

/// Pack the patch parameter data into a message buffer
/// TODO : Pack only subparam (with size)
int CPatch::PackParams(char *buffer)
{
	// Clear patch parameter data buffer
	memset(buffer, 0, PADDR_END);
	strcpy(buffer, m_name);
	char *p = buffer + PADDR_MIXVOL;
	*p = PackScaledValue(m_mixLevel, 0x7F);
	// pack in the ocillator data
	p = buffer + PADDR_OSC1;
	m_osc1.PackParams(p);
	p = buffer + PADDR_OSC2;
	m_osc2.PackParams(p);
	// pack in LFO and Mod data
	p = buffer + PADDR_LFO;
	//m_LFO.PackParams(p);
	p[0] = (char)m_LFOWaveform;
	p[1] = PackScaledValue(m_LFOFreq / LFO_MAX_FREQ, 0x7F);
	p[2] = PackScaledValue(m_LFODepth, 0x7F);
	// gap here
	// MOD data
	p = buffer + PADDR_MOD1;
	int modLen = m_modulator[0].PackParams(p);
	m_modulator[1].PackParams(p + modLen);
	
	return PADDR_END;				// number of bytes added to the buffer
}

/// Unpack the patch param data from a message and update the patch
/// @param buffer		Patch "packed data" buffer
/// @param address		Patch param address (0 = patch base address)
/// @param size			Size of the packed data
void CPatch::UnpackParams(char *buffer, char address, char size)
{
	char *dataEnd = buffer + size;
	char *p = buffer; 
	if (p < dataEnd && PADDR_NAME == address)
		{
		// Read the patch name
		strcpy(m_name, p);
		p += PADDR_MIXVOL - PADDR_NAME;
		address += PADDR_MIXVOL - PADDR_NAME;
		}
	if (p < dataEnd && PADDR_MIXVOL == address)
		{
		// Read the patch mix vol
		m_mixLevel = UnpackValue(*p, 0x7F);
		p += PADDR_OSC1 - PADDR_MIXVOL;
		address += PADDR_OSC1 - PADDR_MIXVOL;
		}
	if (p < dataEnd && PADDR_OSC1 == address)
		{
		// Read the patch OSC1 settings
		m_osc1.UnpackParams(p);
		p += PADDR_OSC2 - PADDR_OSC1;
		address += PADDR_OSC2 - PADDR_OSC1;
		}
	if (p < dataEnd && PADDR_OSC2 == address)
		{
		// Read the patch OSC2 settings
		m_osc2.UnpackParams(p);
		p += PADDR_LFO - PADDR_OSC2;
		address += PADDR_LFO - PADDR_OSC2;
		}
	if (p < dataEnd && PADDR_LFO == address)
		{
		// Read the patch LFO settings
		//m_LFO.UnpackParams(p);
		m_LFOWaveform = (WAVETYPE)p[0];
		//printf("Patch: LFOfreq = %d\n", p[1]);
		m_LFOFreq = UnpackValue(p[1], 0x7F) * LFO_MAX_FREQ;
		m_LFODepth = UnpackValue(p[2], 0x7F);
		// gap here
		p += PADDR_MOD1 - PADDR_LFO;
		address += PADDR_MOD1 - PADDR_LFO;
		}
	if (p < dataEnd && PADDR_MOD1 == address)
		{
		// Read the patch MOD settings
		m_modulator[0].UnpackParams(p);
		p += PADDR_MOD2 - PADDR_MOD1;
		m_modulator[0].UnpackParams(p);
		p += PADDR_END - PADDR_MOD2;
		address += PADDR_END - PADDR_MOD1;
		}
}


