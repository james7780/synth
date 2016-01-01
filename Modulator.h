// Modulation of voice output
#pragma once

enum MODTARGET {
	MT_VOLUME,			// 0
	MT_PITCH,				// 1
	MT_PWM,					// 2
	MT_FILTER				// 3
	};

class CModulator
{
public:
	CModulator();
	~CModulator();

	MODTARGET m_modTarget;
	char m_modRange;
	
	void CopyFrom(const CModulator &rhs);
	/// Pack the modulator parameter data into a message buffer
	int PackParams(char *buffer);
	/// Unpack the modulator param data from a message
	void UnpackParams(char *buffer);
	
};

