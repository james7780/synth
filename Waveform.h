/// Class representing a waveform sample
#pragma once

// Number of samples for the simple waveforms
#define NOMINAL_WAVE_SIZE 4096
#define NOISE_WAVE_SIZE 480000			// about 10 seconds

enum WAVETYPE {
	WT_NONE,							// "off"
	WT_SQUARE,
	WT_SAW1,
	WT_SAW2,
	WT_TRIANGLE,
	WT_SINE,
	WT_NOISE,
	WT_CUSTOM,
	WT_MAX };

class CWaveform
{
public:
	CWaveform(WAVETYPE type, float duty = 0.5f);
	~CWaveform();

	void SetWaveform(WAVETYPE type, float duty = 0.5f);
	void MakeSineWave(float duty = 0.5f);
	void MakeSquareWave(float duty = 0.5f);
	void MakeSaw1Wave(float duty = 0.5f);
	void MakeSaw2Wave(float duty = 0.5f);
	void MakeTriangleWave(float duty = 0.5f);
	void MakeNoiseWave();
	void SetCustomWaveData(float *samples, unsigned int numSamples);
	static const char *GetWaveformName(WAVETYPE type);

	WAVETYPE m_type;
	float *m_samples;
	int m_numSamples;
};

