#include <iostream>
#include <fstream>
#include <valarray>
#include <numeric>
#include <cmath>

#include "waves.h"

/* Wave Class Methods */

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
    ys *= pow(2, header.bitsPerSample - 2);
    std::vector<int16_t> data(begin(ys), end(ys));

    // write data to file
    outFile.write((char*) &data[0], header.dataSize);

    outFile.close(); // close the file
}

/* Signal Class Methods */

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

// evaluate funcion for a Sine Signal
std::valarray<double> Sine::evaluate(std::valarray<double> ts)
{
    std::valarray<double> phases = PI2 * freq * ts + off;
    return amp * cos(phases);
}

// evaluate function for Triangle Signal
std::valarray<double> Triangle::evaluate(std::valarray<double> ts)
{
    std::valarray<double> cycles = freq * ts + off / PI2;
    std::valarray<double> frac = cycles.apply(fractional);
    std::valarray<double> ys = abs(frac - 0.5);
    return normalize(unbias(ys), amp);
}

// evaluate function for Saw Signal
std::valarray<double> Saw::evaluate(std::valarray<double> ts)
{
    std::valarray<double> cycles = freq * ts + off / PI2;
    std::valarray<double> frac = cycles.apply(fractional);
    return normalize(unbias(frac), amp); 
}

// evaluate function for Square Signal
std::valarray<double> Square::evaluate(std::valarray<double> ts)
{
    std::valarray<double> cycles = freq * ts + off / PI2;
    std::valarray<double> frac = cycles.apply(fractional);
    return amp * unbias(frac).apply(sign);
}


/* Helper Maths Functions */

// shifting valarray so that its mean is 0
std::valarray<double> unbias(std::valarray<double> ys)
{
    double mean = ys.sum() / ys.size();
    return ys - mean;
}

// normalize valarray so that the max amplitude is + or - amp
std::valarray<double> normalize(std::valarray<double> ys, float amp)
{
    double high = abs(ys.max());
    double low = abs(ys.min());
    return amp * ys / std::max(high, low);
}

// returns the fractional part of a double (like modf without the integer part)
double fractional(double n)
{
    return n - (long)n;
}

// returns 0 if n is 0, -1 if n < 0 or 1 if n > 0
double sign(double n)
{
    if(n == 0) return 0;
    else if(n < 0) return -1;
    return 1;
}