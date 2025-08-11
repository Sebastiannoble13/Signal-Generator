#include <iostream>

#include "waves.h"

int main(int argc, char* argv[])
{
    Sine sine(440, 1.0, 0);
    Wave sinewave = sine.makeWave(44100, 1.0);
    sinewave.toWav("out.wav");
    return 0;
}