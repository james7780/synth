/// Wave synthesis engine

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <assert.h>

#include "common.h"

#define FREQ 220 /* the frequency we want */
//define CHROMATIC_RATIO 1.059463094359295264562

SDL_AudioSpec audioSpec;

CWaveSynth waveSynth;

// audio_pos acts as our t, keeping track of the sample we are up
// to. We store the position outside the callback so we can keep 
// the sine wave produced at the end of one callback and the 
// start of another continuous. Now for the actual callback:
void audioBufferCallback(void* userdata, Uint8* stream, int len) {
	static bool busy = false;
	if (busy)
		{
		printf("audio callback overrun!\n");
		return;	
		}

	len /= 2; // 16 bit samples

	busy = true;
	waveSynth.m_mixer.FillBuffer((short *) stream, len, audioSpec.freq);
	busy = false;
}

// We've explicitly used a mono signed 16 bit format in our 
// callback, so the number of samples is the length in bytes / 2. 
// We go through the buffer given to us by SDL (stream) and fill it with 
// our sine wave values, then subtract the number of samples written from 
// our remaining length.

static void PostMessage(mqd_t mq, char *buffer, int length)
{
	printf("Posting message...\n");
	mq_send(mq, buffer, length, 0);
}

/// Process patch edit messages for the synth
/// @param buffer			Message buffer
/// @param length			Length of the message
static void ProcessPatchSysex(char *buffer, int length, mqd_t mqGUI)
{
	// Update current ("working") patch with patch data in the 
	// sysex message
	if (length < 10)
		return;

	char command = buffer[4];
	char address = buffer[6];	// param "address"
	char size = buffer[7];		// size of data (or data requested)
	if (0x12 == command)			// "DT1" - recieve patch data
		{
		printf("  DT1 patch data received.\n");
		// read and convert the data
		char *p = buffer + 8; 
		waveSynth.m_workPatch.UnpackParams(p, address, size);
		// Propagate relevant changes to the current patch to the 
		// synth engine
		waveSynth.m_mixer.m_LFOWaveform.SetWaveform(waveSynth.m_workPatch.m_LFOWaveform);
		waveSynth.m_mixer.m_LFOFreq = waveSynth.m_workPatch.m_LFOFreq;
		waveSynth.m_mixer.m_LFODepth = waveSynth.m_workPatch.m_LFODepth;
		}
	else if (0x11 == command)		// "RQ1" - patch data requested
		{
		printf("  RQ1 patch data request recieved\n");
		// Use Patch::PackParams() to create data for message to send
		char outBuffer[MSG_MAX_SIZE] = {
			0xF0, 0x7D, 0x01, 0x01, 0x12, 0x00, 0x00, PADDR_END, 0x00, 0xF7
			};
		char *p = outBuffer + 8; 
		int packedBytes = waveSynth.m_workPatch.PackParams(p);
		PostMessage(mqGUI, outBuffer, packedBytes + 10);
		// TODO : append checksum and F7 properly
		}

}

/// Send patch names / info to GUI
static void SendPatchInfo(mqd_t mqGUI, int patchIndex)
{
	if (patchIndex < NUM_PATCHES)
		{
		CPatch &patch = waveSynth.m_patches[patchIndex];
		char outBuffer[MSG_MAX_SIZE] = {
			0xF0, 0x7D, 0x01, 0x01, 0x12, 0x00, 0x00, PADDR_END, 0x00, 0xF7
			};
		char *p = outBuffer + 8; 
		int packedBytes = patch.PackParams(p);
		PostMessage(mqGUI, outBuffer, packedBytes + 10);
		}
}

/// Process messages for the synth
/// @param buffer			Message buffer
/// @param length			Length of the message
static void ProcessMessage(char *buffer, int length, mqd_t mqGUI)
{
	if (length < 1)
		return;
		
	// All messages are MIDI messages
	char command = buffer[0];
	switch (command & 0xF0)
		{
		case 0x80 :			// Note On
		case 0x90 :			// Note Off
			waveSynth.ProcessMessage(buffer, length);
			break;
		case 0xB0 :			// Controller change
			if (102 == buffer[1] && 1 == buffer[2])
				{
				// "Save patch" command?
				waveSynth.StoreWorkPatch(waveSynth.m_currentPatchIndex);
				printf("Saving patches...\n");
				waveSynth.SavePatches();
				}
			if (103 == buffer[1])
				{
				// Requesting patch info
				int patchIndex = buffer[2];
				printf("Patch info %d requested\n", patchIndex);
				SendPatchInfo(mqGUI, patchIndex);
				}
			else
				{
				waveSynth.ProcessMessage(buffer, length);
				}
			break;
		case 0xC0 :			// Patch change
		case 0xE0 :			// Pitch bend
			waveSynth.ProcessMessage(buffer, length);
			break;
		case 0XF0 :			// System / sysex
			// Sysex
			if (0x7D == buffer[1])
				ProcessPatchSysex(buffer, length, mqGUI);
			break;
		}	// end switch
}

#define MIDIDEVICE "/dev/midi1"
pthread_t midiInThread;

/// MIDI device read thread (to prevent blocking on dev read)
void *midiInThreadFunc(void *data)
{
	// state variables
	unsigned char state = 0;
	unsigned char channel = 0;
	unsigned char note = 0;
	unsigned char vel = 0;
	unsigned char cc = 0;
	unsigned char value = 0;
	FILE *pfMIDIIn = fopen(MIDIDEVICE, "r");
	if (pfMIDIIn)
		{
		// MIDI In thread loop
		char buffer[MSG_MAX_SIZE];
		while (true)
			{
			unsigned char c = (unsigned char)fgetc(pfMIDIIn);
			if (0xF8 == c || 0xFE == c)
				continue;
				
			// Update MIDI in state machine
			if (c & 0x80)
				{
				// "new state"
				state = c & 0xF0;
				channel = c & 0x0F;
				//printf("new state: %2X\n", state);
				}
			else
				{
				// process data according to "running state"
				switch (state)
					{
					case 0x80 :	// note off msg, need note
						note = c;
						buffer[0] = 0x80 | channel;
						buffer[1] = note;
						ProcessMessage(buffer, 2, mqd_t(-1));
						state = 0x80;   // continue "note off" running mode?
						break;
					case 0x90 :	// note on msg, need note
						note = c;
						state = 0x91;	// want vol next
						break;
					case 0x91 :	// note on msg, need vel
						vel = c;
						buffer[0] = 0x90 | channel;
						buffer[1] = note;
						buffer[2] = vel;
						ProcessMessage(buffer, 3, mqd_t(-1));
						state = 0x90;		// continue in running "note on" state
						break;
					case 0xA0 :	// 
						break;
					case 0xB0 :	// CC message
						cc = c;
						state = 0xB1;	// want cc param next
						break;
					case 0xB1 :	// cc param 2
						value = c;
						buffer[0] = 0xB0 | channel;
						buffer[1] = cc;
						buffer[2] = value;
						ProcessMessage(buffer, 3, mqd_t(-1));
						state = 0xB0;		// continue in running "CC" state
						break;
					case 0xC0 :
						break;
					case 0xD0 :
						break;
					case 0xE0 :		// Pitch bend
						buffer[0] = 0xE0 | channel;
						buffer[1] = c;
						state = 0xE1;   // wantr byte 2 of pitch bend
						break;
					case 0xE1 :		// Pitch bend 2nd byte
						buffer[2] = c;
						ProcessMessage(buffer, 3, mqd_t(-1));
						state = 0xE0;   // continue in running PB state
						break;
					}	// end case
				}
				
			}
		}
	else
		{
		printf("Failed to open MIDI device '%s'!\n", MIDIDEVICE);
		}
		
	return NULL;
}

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
	char mqbuffer[MSG_MAX_SIZE];
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = MSG_MAX_SIZE;
	attr.mq_msgsize = MSG_MAX_SIZE;
	attr.mq_curmsgs = 0;
	// Create message queue for receieving messages for the synth engine
	mqd_t mqEngine = mq_open(ENGINE_QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);
	assert(mqEngine != -1);
	
	// Create message queue for sending messages to the gui
	mqd_t mqGUI = mq_open(GUI_QUEUE_NAME, O_CREAT | O_WRONLY | O_NONBLOCK, 0644, &attr);
	assert(mqGUI != -1);


 	//SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
 	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER);

	//SDL_SetVideoMode(640, 480, 0, 0);

	audioSpec.freq = SAMPLERATE;
	audioSpec.format = AUDIO_S16;
	audioSpec.channels = 1;
	audioSpec.silence = 0;
	audioSpec.samples = 512;			// 512 seems lowest without distortion
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

/*
	CPatch *patch = &waveSynth.patches[0];
	patch->m_osc1.m_waveType = WT_SQUARE;
	patch->m_osc1.m_envelope[ET_VOLUME].Set(2000.0f, 100.0f, 0.6f, 2000.0f);

	// Enable voice 1 and 2
	CVoice &voice0 = waveSynth.mixer.voice[0];
	voice0.NoteOn(patch, CWaveSynth::CalcMidiNoteFreqency(72), 0.7f);	// Middle C

	CVoice &voice1 = waveSynth.mixer.voice[1];
	patch->m_osc1.m_waveType = WT_SQUARE;
	patch->m_osc1.m_envelope[ET_VOLUME].Set(100.0f, 100.0f, 0.7f, 100.0f);
	voice1.NoteOn(patch, CWaveSynth::CalcMidiNoteFreqency(76), 0.7f);	// E above middle C
*/

  //SDL_PauseAudioDevice(devID, 0); /* play! */
	SDL_PauseAudio(0);

	// Startup beep
	CPatch *patch = &waveSynth.m_workPatch;
	patch->m_osc1.m_waveType = WT_SQUARE;
	patch->m_osc1.m_envelope[ET_VOLUME].Set(0.0f, 100.0f, 0.9f, 100.0f, 0.7f, 2000.0f);
	waveSynth.NoteOn(-1, 72, 0.8f);
	SDL_Delay(500);
	waveSynth.NoteOff(72);

	// Init patches from data on disk
	waveSynth.LoadPatches();
	waveSynth.SelectPatch(0);

	const float bufferPeriodMS = (float)((1000 * audioSpec.samples) / audioSpec.freq);
	//float bufferPeriodMS = 100.0f;
	printf("Buffer period %.3f ms\n", bufferPeriodMS);

	// Start up midi device input thread
	if (-1 == pthread_create(&midiInThread, NULL, midiInThreadFunc, NULL))
		printf("Error: could not create MIDI device in thread!\n");

	bool done = false;
	while(!done)
		{
		SDL_Delay(10); //(Uint32)bufferPeriodMS);
		// Update all active voices
		waveSynth.Update(10); //bufferPeriodMS);

		// Check for a client request
		ssize_t bytes_read = mq_receive(mqEngine, mqbuffer, MSG_MAX_SIZE, NULL);
		mqbuffer[bytes_read] = 0;
		if (bytes_read > 0)
			{
			printf("mqueue msg received: %s\n", mqbuffer);
			ProcessMessage(mqbuffer, bytes_read, mqGUI);
			}
		
		}

	printf("Finished.\n");
	SDL_CloseAudio();

	// Save patch data to disk
	waveSynth.SavePatches();

	// Tidy up mqueue stuff
	mq_close(mqEngine);
	mq_close(mqGUI);
	mq_unlink(ENGINE_QUEUE_NAME);
	mq_unlink(GUI_QUEUE_NAME);

	return 0;
}
