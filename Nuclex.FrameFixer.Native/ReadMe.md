FrameFixer
========

This is a small program born out of my frustrations removing interlacing artifacts
from movies with a 3:2 pulldown (telecine).

It is specifically designed to do human-aided detelecining. It is automated as much
as possible, but lets you inspect the result and put the algorithm "back in the groove"
when the telecine algorithm is inconsistent (i.e. when the video was cut after already
being telecined).

Because I wanted to keep complexity low, it works on a folder of PNG files. A 90 minute
movie, stored as 48 bit PNG files (16 bit color channels) in standard NTSC resolution
takes up about 100 GiB of disk space.


What is Telecine?
-----------------

Telecine is a way to turn 24 FPS movies into 30 FPS movies from the pre-computer era.
Instead of just displaying every 4. frame twice (1, 2, 3, 4, 4 ... 5, 6, 7, 8, 8 ...),
it uses interlacing to create two mixed images:

    Frame 1 (full)
    Frame 2 (full)
    Frame 2 (odd lines) + Frame 3 (even lines)
    Frame 3 (odd lines) + Frame 4 (even lines)
    Frame 4 (full)

This may or may not have appeared more smoothly on an interlaced NTSC television set.
Unfortunately, NTSC and telecine were still alive and well at the dawn of the DVD era,
so all DVD players must support interlacing and it is widely used.


What about VapourSynth?
-----------------------

My first attempt was to just use VapourSynth with QTGMC and vivtc. VapourSynth lets
you set up a frame processing pipeline for a movie in Python and p

QTGMC is considered the best deinterlacing filter and it digs really deep:
comb detection, motion vectors, frame averaging, artifact fixing, resharpening and more.

https://github.com/HomeOfVapourSynthEvolution/havsfunc/blob/f11d79c98589c9dcb5b10beec35b631db68b495c/havsfunc/havsfunc.py#L1912

And vivtc specifically handles telecine. As far as I understand it, its VFM  tries to
keep on top of the telecine rhythm and can pull deinterlaced frames from QTGMC as needed.

https://github.com/vapoursynth/vivtc/blob/master/docs/vivtc.rst

The result is still 30 FPS, so as the last step, its VDecimate would be used to delete one
in every 6 frames. Unfortunately, it deletes the frame with the least change, which means
that anywhere VFM and/or QTGMC get it wrong, the ugly mess of two mixed interlaced frames
is amongst the frames that remain. This happens particularly often with anime, where many
frames are duplicates (and supposed to be) because the artists animated at 12-15 FPS.

In short, QTGMC + vivtc are excellent for 