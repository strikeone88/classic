/*
    AUDIO.H

    Audio Play Back Engine Version 0.01

    Copyright (C) 2008 Novak Laboratories
    Written by J. Palencia (ilina@bloodykisses.zzn.com)
*/

#ifndef __AUDIO_H
#define __AUDIO_H

    /* Defines the maximum number of sound files to load. */
    #define MAX_SOUNDS  64

    /* Maximum number of sounds to mix. */
    #define MAX_MIX     16

    /* Starts the audio engine returns 1 if no SB and 2 if no XMS. */
    int StartAudio (int base, int irq, int dma);

    /* Stops the audio engine and releases XMS blocks. */
    void StopAudio (void);

    /* Loads a sound file and returns an index. */
    unsigned LoadSound (char *);

    /* Starts playing the given sound. */
    void PlaySound (unsigned i);

    /* Disables the PlaySound function. */
    void disablePlaySound (void);

    /* Enables the PlaySound function. */
    void enablePlaySound (void);

#endif
