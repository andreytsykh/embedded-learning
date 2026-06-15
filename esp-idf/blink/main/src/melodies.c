#include "melodies.h"

// Super Mario — ігрові
static const note_t super_mario[] = {
    {NOTE_E5, 150}, {NOTE_E5, 150}, {NOTE_REST, 150}, {NOTE_E5, 150},
    {NOTE_REST, 150}, {NOTE_C5, 150}, {NOTE_E5, 150}, {NOTE_REST, 150},
    {NOTE_G5, 300}, {NOTE_REST, 300}, {NOTE_G,  300}, {NOTE_REST, 300},
    {NOTE_C5, 300}, {NOTE_REST, 150}, {NOTE_G,  150}, {NOTE_REST, 300},
    {NOTE_E, 300},  {NOTE_REST, 150}, {NOTE_A,  150}, {NOTE_REST, 150},
    {NOTE_B, 150},  {NOTE_REST, 150}, {NOTE_A,  150}, {NOTE_REST, 150},
    {NOTE_A5, 150}, {NOTE_G5, 150},  {NOTE_E5, 150},  {NOTE_G5, 150},
    {NOTE_A5, 300}, {NOTE_F5, 150},  {NOTE_G5, 300},  {NOTE_REST, 150},
    {NOTE_E5, 300}, {NOTE_C5, 150},  {NOTE_D5, 150},  {NOTE_B,  300},
};

// Турецький марш — Моцарт
static const note_t turkish_march[] = {
    {NOTE_E, 150},  {NOTE_D, 150},  {NOTE_C, 150},  {NOTE_D, 150},
    {NOTE_E, 150},  {NOTE_REST, 150},
    {NOTE_E, 150},  {NOTE_D, 150},  {NOTE_C, 150},  {NOTE_D, 150},
    {NOTE_E, 150},  {NOTE_REST, 150},
    {NOTE_A, 300},  {NOTE_G, 150},  {NOTE_A, 150},
    {NOTE_B, 300},  {NOTE_REST, 150},
    {NOTE_A, 150},  {NOTE_G, 150},  {NOTE_F, 150},  {NOTE_G, 150},
    {NOTE_A, 300},  {NOTE_REST, 150},
    {NOTE_E5, 150}, {NOTE_D5, 150}, {NOTE_C5, 150}, {NOTE_D5, 150},
    {NOTE_E5, 150}, {NOTE_REST, 150},
    {NOTE_E5, 150}, {NOTE_D5, 150}, {NOTE_C5, 150}, {NOTE_D5, 150},
    {NOTE_E5, 150}, {NOTE_REST, 150},
    {NOTE_A5, 300}, {NOTE_G5, 150}, {NOTE_A5, 150},
    {NOTE_B, 300},  {NOTE_A, 150},  {NOTE_G, 150},
    {NOTE_A, 600},  {NOTE_REST, 300},
    {NOTE_C5, 150}, {NOTE_B, 150},  {NOTE_A, 150},  {NOTE_B, 150},
    {NOTE_C5, 300}, {NOTE_REST, 150},
    {NOTE_C5, 150}, {NOTE_B, 150},  {NOTE_A, 150},  {NOTE_B, 150},
    {NOTE_C5, 300}, {NOTE_REST, 150},
    {NOTE_E5, 300}, {NOTE_D5, 150}, {NOTE_C5, 150},
    {NOTE_B, 300},  {NOTE_A, 150},  {NOTE_G, 150},
    {NOTE_A, 600},  {NOTE_REST, 300},
};

// Smoke on the Water — Deep Purple
static const note_t smoke_on_water[] = {
    {NOTE_G, 300},  {NOTE_A, 300},  {NOTE_C5, 450}, {NOTE_REST, 150},
    {NOTE_G, 300},  {NOTE_A, 300},  {NOTE_D5, 200}, {NOTE_C5, 400},
    {NOTE_G, 300},  {NOTE_A, 300},  {NOTE_C5, 450}, {NOTE_REST, 150},
    {NOTE_A, 300},  {NOTE_G, 600},  {NOTE_REST, 300},
    {NOTE_G, 300},  {NOTE_A, 300},  {NOTE_C5, 450}, {NOTE_REST, 150},
    {NOTE_G, 300},  {NOTE_A, 300},  {NOTE_D5, 200}, {NOTE_C5, 400},
    {NOTE_G, 300},  {NOTE_A, 300},  {NOTE_C5, 300}, {NOTE_A, 300},
    {NOTE_G, 600},  {NOTE_REST, 300},
};

// We Wish You a Merry Christmas — різдвяні
static const note_t we_wish[] = {
    {NOTE_C, 300},
    {NOTE_F, 300},  {NOTE_F, 150},  {NOTE_G, 150},  {NOTE_F, 150},  {NOTE_E, 150},
    {NOTE_D, 300},  {NOTE_D, 300},  {NOTE_D, 300},
    {NOTE_G, 300},  {NOTE_G, 150},  {NOTE_A5, 150}, {NOTE_G5, 150}, {NOTE_F5, 150},
    {NOTE_E5, 300}, {NOTE_C, 300},  {NOTE_C, 300},
    {NOTE_A5, 300}, {NOTE_A5, 150}, {NOTE_B, 150},  {NOTE_A5, 150}, {NOTE_G5, 150},
    {NOTE_F5, 300}, {NOTE_D5, 300}, {NOTE_C, 150},  {NOTE_C, 150},
    {NOTE_D5, 300}, {NOTE_G5, 300}, {NOTE_E5, 300},
    {NOTE_F5, 600}, {NOTE_REST, 300},
    {NOTE_C5, 300}, {NOTE_C5, 300}, {NOTE_C5, 300},
    {NOTE_D5, 450}, {NOTE_C5, 150}, {NOTE_B, 600},
};

// Tetris
static const note_t tetris[] = {
    {NOTE_E5, 300}, {NOTE_B, 150},  {NOTE_C5, 150}, {NOTE_D5, 300},
    {NOTE_C5, 150}, {NOTE_B, 150},  {NOTE_A, 300},  {NOTE_A, 150},
    {NOTE_C5, 150}, {NOTE_E5, 300}, {NOTE_D5, 150}, {NOTE_C5, 150},
    {NOTE_B, 450},  {NOTE_C5, 150}, {NOTE_D5, 300}, {NOTE_E5, 300},
    {NOTE_C5, 300}, {NOTE_A, 300},  {NOTE_A, 600},  {NOTE_REST, 150},
    {NOTE_D5, 300}, {NOTE_F5, 150}, {NOTE_A5, 300}, {NOTE_G5, 150},
    {NOTE_F5, 150}, {NOTE_E5, 450}, {NOTE_C5, 150}, {NOTE_E5, 300},
    {NOTE_D5, 150}, {NOTE_C5, 150}, {NOTE_B, 300},  {NOTE_B, 150},
    {NOTE_C5, 150}, {NOTE_D5, 300}, {NOTE_E5, 300},
    {NOTE_C5, 300}, {NOTE_A, 300},  {NOTE_A, 600},
};

const melody_t melodies[] = {
    {super_mario,    sizeof(super_mario)    / sizeof(note_t), "Super Mario"},
    {turkish_march,  sizeof(turkish_march)  / sizeof(note_t), "Turkish March"},
    {smoke_on_water, sizeof(smoke_on_water) / sizeof(note_t), "Smoke on the Water"},
    {we_wish,        sizeof(we_wish)        / sizeof(note_t), "We Wish You a Merry Christmas"},
    {tetris,         sizeof(tetris)         / sizeof(note_t), "Tetris"},
};

const int melodies_count = sizeof(melodies) / sizeof(melody_t);