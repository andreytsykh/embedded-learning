// melodies.h
#pragma once

#include <stdint.h>

typedef enum {
    NOTE_REST = 0,
    NOTE_C  = 262, NOTE_D  = 294, NOTE_E  = 330, NOTE_F  = 349,
    NOTE_G  = 392, NOTE_A  = 440, NOTE_B  = 494, NOTE_C5 = 523,
    NOTE_D5 = 587, NOTE_E5 = 659, NOTE_F5 = 698, NOTE_G5 = 784,
    NOTE_A5 = 880,
} note_freq_t;

typedef struct {
    note_freq_t freq;
    uint16_t    duration_ms;
} note_t;

typedef struct {
    const note_t *notes;
    int           len;
    const char   *name;
} melody_t;

extern const melody_t melodies[];
extern const int      melodies_count;