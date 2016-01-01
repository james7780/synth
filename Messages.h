/// Synth Engine Messages

#define MSG_MAX_SIZE	200

// Parameter addresses
#define PADDR_NAME			0				// 20 bytes
#define PADDR_MIXVOL		20				// 1 byte + padding
#define PADDR_OSC1			22				// 52 bytes
#define PADDR_OSC2			74				// 52
#define PADDR_LFO			126				// 19 bytes + padding
#define PADDR_MOD1			146				// 3 bytes + padding
#define PADDR_MOD2			150				// 3 bytes + padding
#define PADDR_END			154

/// Pack a float value scaled to { 0.0 - 1.0 } to a byte value  
char PackScaledValue(float value, char maxVal);

/// Unpack a btye value between 0 and maxVal to a float between 0.0 and 1.0
float UnpackValue(char data, char maxVal);
