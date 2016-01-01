#include "Modulator.h"
#include "Messages.h"

CModulator::CModulator()
	: m_modTarget(MT_VOLUME),
		m_modRange(0)
{
}

CModulator::~CModulator()
{
}

void CModulator::CopyFrom(const CModulator &rhs)
{
	m_modTarget = rhs.m_modTarget;
	m_modRange = rhs.m_modRange;
}

/// Pack the modulator parameter data into a message buffer
int CModulator::PackParams(char *buffer)
{
	buffer[0] = m_modRange;
	buffer[1] = (char)m_modTarget;
	
	return PADDR_MOD2 - PADDR_MOD1;	// number of bytes added to the buffer
}

/// Unpack the modulator param data from a message and update
/// the modulator
void CModulator::UnpackParams(char *buffer)
{
	m_modRange = buffer[0];
	m_modTarget = (MODTARGET)buffer[1];
}
