/// Stuff common to both the engine and the GUI

//#include "SDL.h"
#include <SDL2/SDL.h>
#include "Voice.h"
#include "Mixer.h"
#include "WaveSynth.h"
#include "Messages.h"
#ifdef WIN32
#include "mqueue-w32-master\mqueue.h"
#else
#include "mqueue.h"
#endif

#define ENGINE_QUEUE_NAME "/synthqueue"
#define GUI_QUEUE_NAME "/synthguiqueue"

#define SAMPLERATE	48000

