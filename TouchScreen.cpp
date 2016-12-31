/// RPi (Linux) touchscreen handler
/// Copyright James Higgs 2016
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <SDL2/SDL.h>

// global!
static int fdTouch = 0;
	
/// Open RPi offical touchscreen divice for touch events
int Touch_Open()
{
	// Enumerate to find the correct event input
	char devPath[64] = "/dev/input/event0";
	for (int i = 0; i < 4; i++)
		{
		devPath[strlen(devPath) - 1] = 48 + i;   // '0' to '3'
		int fd = open(devPath, O_RDONLY | O_NONBLOCK);
		if (fd)
			{
			char name[256] = "Unknown";
			ioctl(fd, EVIOCGNAME(sizeof(name)), name);
			if (0 == strncmp(name, "FT5406", 6))
				{
				printf("Touch device name: %s\n", name);
				fdTouch = fd;
				break;
				}
			else
				close(fd);
			}
		}
	
	if (0 == fdTouch)
		printf("Error: Touch device FT5406 not found!\n");
		
	return fdTouch;
}

/// Read touch info
// struct input_event:
//    __u16 type     (eg:EV_SYN, EV_REL, EV_ABS)
//    __u16 code     (eg: REL_X, REL_Y)
//    __s32 value    (the X/Y etc value)
// Touch down: type = EV_KEY and code = BTN_TOUCH and value = 1
// Touch up: type = EV_KEY and code = BTN_TOUCH and value = 0
// Touch X: type = EV_ABS, code = 0, value > -1 = X
// Touch Y: type = EV_ABS, code = 1, value > -1 = Y

struct input_event touchEvents[64];
int touchX = 0;
int touchY = 0;
void Touch_Update()
{
	if (fdTouch)
		{
		// Note: reading events here will cause SDL events to not work
		// (uses the same events interface?)
		int bytes = read(fdTouch, touchEvents, sizeof(struct input_event)*64);
		if (bytes > 0)
			{
			for (unsigned int i = 0; i < (bytes / sizeof(struct input_event)); i++)
				{
				__u16 type = touchEvents[i].type;
				__u16 code = touchEvents[i].code;
				__s32 value = touchEvents[i].value;
				printf("ET: %d   EC:  %d   EV: %d\n", type, code, value);
				
				// Push touch events to SDL event queue, as mouse events
				if (EV_KEY == type && BTN_TOUCH == code)
					{
					if (1 == value || 0 == value)	// touch down/up
						{
						// Send touch up/down event to SDL2
						SDL_Event sdlEvent;
						sdlEvent.type = (1 == value) ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
						sdlEvent.button.x = touchX;	// use "current" mouse x/y set by EV_ABS code 53/54 below
						sdlEvent.button.y = touchY;
						SDL_PushEvent(&sdlEvent);	
						}
					}
				else if (EV_ABS == type)
					{
					if (53 == code)							// mouse down x
						{
						touchX = value;
						}
					else if (54 == code)					// mouse down y
						{
						touchY = value;
						}							
					if (0 == code)							// mouse/touch move x
						{
						touchX = value;
						}
					else if (1 == code)						// mouse/touch move y
						{
						touchY = value;
						// Send mouse move event to SDL2
						SDL_Event sdlEvent;
						sdlEvent.type = SDL_MOUSEMOTION;
						sdlEvent.button.x = touchX;
						sdlEvent.button.y = touchY;
						SDL_PushEvent(&sdlEvent);
						}
					}
				}
			}
		}
	
}

/// Open RPi offical touchscreen divice for touch events
void Touch_Close()
{
	close(fdTouch);
}

