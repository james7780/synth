/// Test code to play with wave synthesis

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include "SDL.h"
#include "Voice.h"
#include "Mixer.h"
#include "WaveSynth.h"
//#include "Windows.h"

#include <mqueue.h>
#define MQUEUE_NAME "/synthqueue"

#define FREQ 220 /* the frequency we want */
#define CHROMATIC_RATIO 1.059463094359295264562

SDL_AudioSpec audioSpec;

CWaveSynth waveSynth;

// audio_pos acts as our t, keeping track of the sample we are up
// to. We store the position outside the callback so we can keep 
// the sine wave produced at the end of one callback and the 
// start of another continuous. Now for the actual callback:
void audioBufferCallback(void* userdata, Uint8* stream, int len) {
  len /= 2; /* 16 bit */

	waveSynth.mixer.FillBuffer((short *) stream, len, audioSpec.freq);
}

// We've explicitly used a mono signed 16 bit format in our 
// callback, so the number of samples is the length in bytes / 2. 
// We go through the buffer given to us by SDL (stream) and fill it with 
// our sine wave values, then subtract the number of samples written from 
// our remaining length.

// To handle KILL signal
volatile sig_atomic_t g_done = 0;
void terminate(int signum)
{
	g_done = 1;
}

int main(int argc, char *argv[])
{
	struct sigaction killaction;
	memset(&killaction, 0, sizeof(struct sigaction));
	killaction.sa_handler = terminate;
	sigaction(SIGTERM, &killaction, NULL);
	
	// IPC message queue stuff
	char mqbuffer[10];
	mqd_t mq;
	struct mq_attr attr;
	// init queue attributes
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = 10;
	attr.mq_curmsgs = 0;
	// create message queue
	mq = mq_open(MQUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);
	assert(mq != -1);

 	//SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
 	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER);

	//SDL_SetVideoMode(640, 480, 0, 0);

	audioSpec.freq = 48000;
	audioSpec.format = AUDIO_S16;
	audioSpec.channels = 1;
	audioSpec.silence = 0;
	audioSpec.samples = 4096;
	audioSpec.callback = audioBufferCallback;
 	audioSpec.userdata = NULL;

  //SDL_AudioDeviceID devID = SDL_OpenAudioDevice(NULL, 0, &want, &audioSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	//if (0 == devID)
	//	{
    //printf("\nFailed to open audio: %s\n", SDL_GetError());
    //return 1;
//		}
	if (SDL_OpenAudio(&audioSpec, NULL) < 0)
		{
		printf("Failed to open audio: %s\n", SDL_GetError());
		return 1;
		}
	printf("Audio opened successfully:\n");
	printf("   Freq = %d\n", audioSpec.freq);
	printf("   Samples = %d.\n", audioSpec.samples);
	printf("   Channels = %d.\n", audioSpec.channels);
	printf("   Format = %X\n", audioSpec.format);

	// Now all that's left to do is set up the global variables, and play the 
	// audio. When you open a device, the audio is automatically paused, so 
	// playing is just a matter of unpausing it. We wait 500 ms at a time 
	// until we've played audio_len samples, then we can just close the device 
	// and exit normally.

	CPatch *patch = &waveSynth.patches[0];
	patch->m_osc1.m_waveType = WT_SINE;
	patch->m_osc1.m_envelope[ET_VOLUME].Set(2000.0f, 100.0f, 0.6f, 2000.0f);

	// Enable voice 1 and 2
	CVoice &voice0 = waveSynth.mixer.voice[0];
	voice0.NoteOn(patch, CWaveSynth::CalcMidiNoteFreqency(72), 0.7f);	// Middle C

	CVoice &voice1 = waveSynth.mixer.voice[1];
	patch->m_osc1.m_waveType = WT_SQUARE;
	patch->m_osc1.m_envelope[ET_VOLUME].Set(100.0f, 100.0f, 0.7f, 100.0f);
	voice1.NoteOn(patch, CWaveSynth::CalcMidiNoteFreqency(76), 0.7f);	// E above middle C


  //SDL_PauseAudioDevice(devID, 0); /* play! */
	SDL_PauseAudio(0);

	const float bufferPeriodMS = (float)((1000 * audioSpec.samples) / audioSpec.freq);
	//float bufferPeriodMS = 100.0f;
	printf("Buffer period %.3f ms\n", bufferPeriodMS);

	bool done = false;
	while(!done)
		{
		SDL_Delay((Uint32)bufferPeriodMS);
		// Update all active voices
		waveSynth.Update(bufferPeriodMS);

		// Check for a client request
		ssize_t bytes_read = mq_receive(mq, mqbuffer, 10, NULL);
		mqbuffer[bytes_read] = 0;
		if (bytes_read > 0)
			{
			printf("mqueue msg received: %s\n", mqbuffer);
			waveSynth.ProcessMessage(mqbuffer, 10);
			}
			
		}

	printf("Finished.\n");
	SDL_CloseAudio();

	// Tidy up mqueue stuff
	mq_close(mq);
	mq_unlink(MQUEUE_NAME);

	return 0;
}
