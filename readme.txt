Building "synth" on Raspbian:

1. Install libsdl2-dev (included in latest jessie packages)

Uses mqueue (built-in POSIX), should be default part of Raspbian installation.

import posix_ipc  (for Python)
#include <mqueue.h>  (for C)

To set up audio output from RPi audio jack ("analog audio"):
1. Ensure that you have “snd_bcm2835” line in “/etc/modules” file.
2. Ensure that “/usr/share/alsa/alsa.conf” file has following update:
      - Change the line “pcm.front cards.pcm.front” to “pcm.front cards.pcm.default” in this file.
3. amixer cset numid=3 1   (1 = analog, 2 = hdmi, 0 = auto)
4. reboot
5. speaker-test -c2

To build:
cd synth
make clean
make -f Makefile.gui clean
make
make -f Makefile.gui

(Sorry, still need to update makefile to build both targets at the same time).

To run:
Run "synth" executable in one terminal window
Run "synthgui" in another terminal window
You will need to run using sudo
The synthgui is hardcoded to run at RPF Official Touchscreen resolution (800x480).
Synth is hardcoded to use "/dev/midi1" for attached MIDI controller connected to your RPi. 

- JH


