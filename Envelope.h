// ADSR envelope for shaping voice output
#pragma once

enum ENVTYPE {
	ET_VOLUME,				// 0
	ET_PITCH,				// 1
	ET_PWM,					// 2
	ET_FILTER,				// 3
	ET_MOD,					// 4
	ET_MAX
	};

// Max time for attack or decay or release
#define ENV_MAX_ATTACK	10000.0f

class CEnvelope
{
public:
	CEnvelope();
	CEnvelope(float delay, float a, float p, float d, float s, float r);
	~CEnvelope();

	void Set(float delay, float a, float p, float d, float s, float r);
	float GetLevel(float time, bool released) const;

	float m_delay;				// in ms
	float m_attack;				// in ms
	float m_peak;				// attack peak - 0.0 to 1.0
	float m_decay;				// in ms
	float m_sustain;			// level - 0.0 to 1.0
	float m_release;			// in ms
	
	void CopyFrom(const CEnvelope &rhs);
	/// Pack the patch parameter data into a message buffer
	int PackParams(char *buffer);
	/// Unpack the envelope param data from a message
	void UnpackParams(char *buffer);
};

