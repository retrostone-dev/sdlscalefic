SDLscale fix is a small library for allowing easy ports from the GCW0, Dingux and similar to the Arcade Mini.
It is based on sdlfix, which was a similar library for the RS-97, with a slightly different purpose.

It intercepts the video calls and upscales them to 680x448, which is going to be the resolution used for the Retrostone.

Usage
======

Use it with LD_PRELOAD and load your app with it.

#!/bin/sh
LD_PRELOAD=./sdlfix.so ./DinguxCommander.dge

Make sure said app is dynamic linked to libSDL.so.
