#include <iostream>

#include "waves.h"

int main(int argc, char* argv[])
{
    // check for command line arguments
    if(argc < 4 || argc > 4)
    {
        throw std::invalid_argument("Usage ./signals frequency amplitude offset\n");
    }

    Sine sine(atoi(argv[1]), atof(argv[2]), atof(argv[3]));
    Sine mod(10, 1, 0);
    sine.addMod(0, &mod, 1);
    Wave sinewave = sine.makeWave(44100, 1);
    sinewave.toWav("out.wav");
    return 0;
}