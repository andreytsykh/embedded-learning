#include <Arduino.h>
#include "blink.h"

void setup()
{
    init();
}

void loop()
{
    patternStrobe(400);
}
