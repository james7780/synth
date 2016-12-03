1. Install libsdl2-dev

mqueue packages
import posix_ipc  (for Python)
#include <mqueue.h>  (for C)

Rather use mqueue (built-in POSIX) than zeromq (3rd-party)?


Analog audio output:
1. “snd_bcm2835” line in “/etc/modules” file.
2. Ensure that “/usr/share/alsa/alsa.conf” file has following update:
      - Change the line “pcm.front cards.pcm.front” as “pcm.front cards.pcm.default” in this file.
3. amixer cset numid=3 1   (1 = analog, 2 = hdmi, 0 = auto)
4. reboot
5. speaker-test -c2

