/// Messages
#include "Messages.h"

/// Pack a float value scaled to { 0.0 - 1.0 } to a byte value  
char PackScaledValue(float value, char maxVal)
{
	return (char)(value * maxVal);
}

/// Unpack a byte value to a float value
float UnpackValue(char data, char maxVal)
{
	return (float)data / maxVal;
	
}
