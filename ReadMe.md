  - **Status:** Working and Usable but still in development and prone to
    sudden changes, including compatibility breaks on data files.

  - **Platforms:** Cross-platform, developed on Linux, will eventually
    receive testing on Windows.


Frame Fixer
===========

This is a deinterlacing and detelecining (aka inverse telecine) tool of last resort.

What makes it a *tool of last resort*? Frame Fixer's work begins where VapourSynth and QTGMC throw
the towel - it lets you do a manual detelecine. That means you can dump your entire movie into
a folder (about 75 GiB per hour of 29.98 fps NTSC) like this:

    mkdir frames
    ffmpeg \
    	-i movie-ntsc-dvd-remux.mkv \
    	-f image2 -pix_fmt rgb48be \
    	-vsync 0 \
    	frames/%08d.png

And then you can open that directory in Frame Fixer, which lets you inspect and tag each movie
frame on its own to decide whether it should be kept as-is, deinterlaced from the top field,
bottom field, top-first or bottom-first.

The main workflow is non-destructive (you only tag stuff until you click render) and made as
convenient as possible (you can work with just the arrow keys and a hand on the left side of
the keyboard), but let's not fool ourselves - fixing an entire movie is a ton of work.

![Frame Fixer Main Window](./Nuclex.FrameFixer.Native/Documents/frame-fixer-main-window.png)
