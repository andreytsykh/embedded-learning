#include "melodies.h"

static const note_t super_mario[] = {
    {NOTE_E5, 150U}, {NOTE_E5, 150U}, {NOTE_REST, 150U}, {NOTE_E5, 150U},
    {NOTE_REST, 150U}, {NOTE_C5, 150U}, {NOTE_E5, 150U}, {NOTE_REST, 150U},
    {NOTE_G5, 300U}, {NOTE_REST, 300U}, {NOTE_G,  300U}, {NOTE_REST, 300U},
    {NOTE_C5, 300U}, {NOTE_REST, 150U}, {NOTE_G,  150U}, {NOTE_REST, 300U},
    {NOTE_E, 300U},  {NOTE_REST, 150U}, {NOTE_A,  150U}, {NOTE_REST, 150U},
    {NOTE_B, 150U},  {NOTE_REST, 150U}, {NOTE_A,  150U}, {NOTE_REST, 150U},
    {NOTE_A5, 150U}, {NOTE_G5, 150U},  {NOTE_E5, 150U}, {NOTE_G5, 150U},
    {NOTE_A5, 300U}, {NOTE_F5, 150U},  {NOTE_G5, 300U}, {NOTE_REST, 150U},
    {NOTE_E5, 300U}, {NOTE_C5, 150U},  {NOTE_D5, 150U}, {NOTE_B,  300U},
};

static const note_t turkish_march[] = {
    {NOTE_E, 150U},  {NOTE_D, 150U},  {NOTE_C, 150U},  {NOTE_D, 150U},
    {NOTE_E, 150U},  {NOTE_REST, 150U},
    {NOTE_E, 150U},  {NOTE_D, 150U},  {NOTE_C, 150U},  {NOTE_D, 150U},
    {NOTE_E, 150U},  {NOTE_REST, 150U},
    {NOTE_A, 300U},  {NOTE_G, 150U},  {NOTE_A, 150U},
    {NOTE_B, 300U},  {NOTE_REST, 150U},
    {NOTE_A, 150U},  {NOTE_G, 150U},  {NOTE_F, 150U},  {NOTE_G, 150U},
    {NOTE_A, 300U},  {NOTE_REST, 150U},
    {NOTE_E5, 150U}, {NOTE_D5, 150U}, {NOTE_C5, 150U}, {NOTE_D5, 150U},
    {NOTE_E5, 150U}, {NOTE_REST, 150U},
    {NOTE_A5, 300U}, {NOTE_G5, 150U}, {NOTE_A5, 150U},
    {NOTE_B, 300U},  {NOTE_A, 150U},  {NOTE_G, 150U},
    {NOTE_A, 600U},  {NOTE_REST, 300U},
};

static const note_t smoke_on_water[] = {
    {NOTE_G, 300U}, {NOTE_A, 300U}, {NOTE_C5, 450U}, {NOTE_REST, 150U},
    {NOTE_G, 300U}, {NOTE_A, 300U}, {NOTE_D5, 200U}, {NOTE_C5, 400U},
    {NOTE_G, 300U}, {NOTE_A, 300U}, {NOTE_C5, 300U}, {NOTE_A, 300U},
    {NOTE_G, 600U}, {NOTE_REST, 300U},
};

static const note_t we_wish[] = {
    {NOTE_C, 300U},
    {NOTE_F, 300U},  {NOTE_F, 150U},  {NOTE_G, 150U},  {NOTE_F, 150U}, {NOTE_E, 150U},
    {NOTE_D, 300U},  {NOTE_D, 300U},  {NOTE_D, 300U},
    {NOTE_G, 300U},  {NOTE_G, 150U},  {NOTE_A5, 150U}, {NOTE_G5, 150U}, {NOTE_F5, 150U},
    {NOTE_E5, 300U}, {NOTE_C, 300U},  {NOTE_C, 300U},
    {NOTE_F5, 600U}, {NOTE_REST, 300U},
};

static const note_t tetris[] = {
    {NOTE_E5, 300U}, {NOTE_B, 150U},  {NOTE_C5, 150U}, {NOTE_D5, 300U},
    {NOTE_C5, 150U}, {NOTE_B, 150U},  {NOTE_A, 300U},  {NOTE_A, 150U},
    {NOTE_C5, 150U}, {NOTE_E5, 300U}, {NOTE_D5, 150U}, {NOTE_C5, 150U},
    {NOTE_B, 450U},  {NOTE_C5, 150U}, {NOTE_D5, 300U}, {NOTE_E5, 300U},
    {NOTE_C5, 300U}, {NOTE_A, 300U},  {NOTE_A, 600U},  {NOTE_REST, 150U},
};

static const note_t safe_tick_notes[] = {
    {2600, 20U},
    {NOTE_REST, 20U}
};

static const note_t safe_unlocked_notes[] = {
    {NOTE_C5, 140U}, {NOTE_E5, 140U}, {NOTE_G5, 140U}, {NOTE_C5, 180U},
    {NOTE_REST, 60U},
    {NOTE_E5, 120U}, {NOTE_G5, 120U}, {NOTE_A5, 160U},
    {NOTE_REST, 50U},
    {NOTE_G5, 120U}, {NOTE_E5, 120U}, {NOTE_C5, 140U},
    {NOTE_REST, 60U},
    {NOTE_D5, 120U}, {NOTE_F5, 120U}, {NOTE_A5, 180U},
    {NOTE_REST, 60U},
    {NOTE_G5, 120U}, {NOTE_A5, 120U}, {NOTE_C5, 220U},
    {NOTE_REST, 80U},
    {NOTE_C5, 100U}, {NOTE_E5, 100U}, {NOTE_G5, 100U}, {NOTE_A5, 260U},
};

static const note_t safe_locked_notes[] = {
    {2400U, 70U}, {NOTE_REST, 25U},
    {900U,  70U}, {NOTE_REST, 25U},
    {2400U, 70U}, {NOTE_REST, 25U},
    {900U,  70U}, {NOTE_REST, 80U},

    {2800U, 55U}, {NOTE_REST, 20U},
    {1100U, 55U}, {NOTE_REST, 20U},
    {2800U, 55U}, {NOTE_REST, 20U},
    {1100U, 55U}, {NOTE_REST, 80U},

    {3200U, 45U}, {NOTE_REST, 15U},
    {1300U, 45U}, {NOTE_REST, 15U},
    {3200U, 45U}, {NOTE_REST, 15U},
    {1300U, 45U}, {NOTE_REST, 100U},

    {3000U, 40U}, {2600U, 40U}, {2200U, 40U}, {1800U, 40U},
    {1400U, 40U}, {1000U, 40U}, {700U,  120U},

    {NOTE_REST, 120U},

    {3300U, 35U}, {NOTE_REST, 15U},
    {3300U, 35U}, {NOTE_REST, 15U},
    {3300U, 35U}, {NOTE_REST, 15U},
    {900U,  180U},

    {NOTE_REST, 180U},
};

const melody_t melodies[] = {
    {super_mario,    sizeof(super_mario)    / sizeof(super_mario[0]),    "Super Mario"},
    {turkish_march,  sizeof(turkish_march)  / sizeof(turkish_march[0]),  "Turkish March"},
    {smoke_on_water, sizeof(smoke_on_water) / sizeof(smoke_on_water[0]), "Smoke on the Water"},
    {we_wish,        sizeof(we_wish)        / sizeof(we_wish[0]),        "We Wish You a Merry Christmas"},
    {tetris,         sizeof(tetris)         / sizeof(tetris[0]),         "Tetris"},
};

const int melodies_count = sizeof(melodies) / sizeof(melodies[0]);

const melody_t safe_tick_melody = {
    .notes = safe_tick_notes,
    .len = sizeof(safe_tick_notes) / sizeof(safe_tick_notes[0]),
    .name = "Safe Tick"
};

const melody_t safe_unlocked_melody = {
    .notes = safe_unlocked_notes,
    .len = sizeof(safe_unlocked_notes) / sizeof(safe_unlocked_notes[0]),
    .name = "Money Vault"
};

const melody_t safe_locked_melody = {
    .notes = safe_locked_notes,
    .len = sizeof(safe_locked_notes) / sizeof(safe_locked_notes[0]),
    .name = "Alarm"
};
