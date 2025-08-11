#include <iostream>
#include <fstream>
#include <valarray>
#include <numeric>
#include <cmath>

#include "waves.h"

// Signal constructor
Signal::Signal(int frequency, float amplitude, float offset)
{
    freq = frequency;
    amp = amplitude;
    off = offset;

}

// Signal evaluate
std::valarray<double> Signal::evaluate(std::valarray<double> ts)
{
    return ts;
}

// convert Signal object to a Wave object
Wave Signal::makeWave(int sampleRate, float duration)
{
    int len = sampleRate * duration;
    std::valarray<double> ts(len);
    std::iota(begin(ts), end(ts), 0);
    ts /= sampleRate;

    std::valarray<double> ys = evaluate(ts);
    return Wave(ys, sampleRate, duration);
}

// evaluate funcion for a sine signal
std::valarray<double> Sine::evaluate(std::valarray<double> ts)
{
    std::valarray<double> phases = PI2 * freq * ts + off;
    std::valarray<double> ys = amp * cos(phases);
    return ys;
}

// Wave object constructor
Wave::Wave(std::valarray<double> samples, int sampleRate, float duration)
{
    ys = samples;
    rate = sampleRate;
    dur = duration;
}

// write the wave to a wav file
void Wave::toWav(std::string filename) 
{
    // open file
    std::ofstream outFile(filename);
    if(!outFile)
    {
        throw std::invalid_argument("Unable to open output file");
    }

    // create header
    wavHeader header;
    header.sampleRate = rate;
    header.bytesPerSec = rate * header.blockSize;

    // data and file size
    header.dataSize = ys.size() * header.blockSize;
    header.fileSize = header.dataSize + sizeof(wavHeader) - 2;

    // write header to file
    outFile.write((char*) &header, sizeof(wavHeader));

    // convert ys to 16 bit integer
    ys *= pow(2, header.bitsPerSample - 1) / 2;
    std::vector<int16_t> data(begin(ys), end(ys));

    // write data to file
    for(int i = 0; i < data.size(); i++)
    {
        outFile.write((char*) &data[i], header.blockSize);
    }

    outFile.close(); // close the file
}