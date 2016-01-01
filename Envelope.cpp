#include "Envelope.h"
#include "Messages.h"

CEnvelope::CEnvelope()
	:	m_delay(0.0f), 
		m_attack(50.0f),
		m_peak(0.9f),
		m_decay(100.0f),
		m_sustain(0.7f),
		m_release(200.0f)
{
}

CEnvelope::CEnvelope(float delay, float a, float p, float d, float s, float r)
	: 	m_delay(delay),
		m_attack(a),
		m_peak(p),
		m_decay(d),
		m_sustain(s),
		m_release(r)
{
}

CEnvelope::~CEnvelope()
{
}

/// Set the ADSR envelope values
/// @param delay		m_delay time in ms
/// @param a			m_attack time in ms
/// @param p			m_peak level (0.0 to 1.0)
/// @param d			Delat time in ms
/// @param s			m_sustain level (0.0 to 1.0)
/// @param r			m_release time (ms)
void CEnvelope::Set(float delay, float a, float p, float d, float s, float r)
{
	m_delay= delay;
	m_attack = a;
	m_peak = p;
	m_decay = d;
	m_sustain = s;
	m_release = r;
}

/// Get the envelope level for the time since note on or note off
/// @param time				Time is ms since note on or note off
/// @param m_released		true if note has been m_released (time since note off)
float CEnvelope::GetLevel(float time, bool m_released) const
{
	float level = m_sustain;
	if (m_released)
		{
		if (time < m_release)
			level = (1.0f - time / m_release) * m_sustain;
		else
			level = 0.0f;
		}
	else
		{
		if (time < m_delay)
			{
			level = 0.0f;
			}
		else
			{
			time -= m_delay;
			if (time < m_delay + m_attack)
				{
				level = m_peak * (time / m_attack);
				}
			else if (time < m_attack + m_decay)
				{
				level = m_peak - ((time - m_attack) / m_decay) * (m_peak - m_sustain); 
				}
			}
		// else level = m_sustain
		}

	return level;
}

void CEnvelope::CopyFrom(const CEnvelope &rhs)
{
	m_delay= rhs.m_delay;
	m_attack = rhs.m_attack;
	m_peak = rhs.m_peak;
	m_decay = rhs.m_decay;
	m_sustain = rhs.m_sustain;
	m_release = rhs.m_release;	
}

/// Pack the envelope parameter data into a message buffer
int CEnvelope::PackParams(char *buffer)
{
	buffer[0] = PackScaledValue(m_delay / ENV_MAX_ATTACK, 0x7F);
	buffer[1] = PackScaledValue(m_attack / ENV_MAX_ATTACK, 0x7F);
	buffer[2] = PackScaledValue(m_peak, 0x7F);
	buffer[3] = PackScaledValue(m_decay / ENV_MAX_ATTACK, 0x7F);
	buffer[4] = PackScaledValue(m_sustain, 0x7F);
	buffer[5] = PackScaledValue(m_release / ENV_MAX_ATTACK, 0x7F);
	
	return 6;	// number of bytes added to the buffer
}

/// Unpack the envelope param data from a message and update
/// the envelope
void CEnvelope::UnpackParams(char *buffer)
{
	m_delay = UnpackValue(buffer[0], 0x7F) * ENV_MAX_ATTACK;
	m_attack = UnpackValue(buffer[1], 0x7F) * ENV_MAX_ATTACK;
	m_peak = UnpackValue(buffer[2], 0x7F);
	m_decay = UnpackValue(buffer[3], 0x7F) * ENV_MAX_ATTACK;
	m_sustain = UnpackValue(buffer[4], 0x7F);
	m_release = UnpackValue(buffer[5], 0x7F) * ENV_MAX_ATTACK;
}
