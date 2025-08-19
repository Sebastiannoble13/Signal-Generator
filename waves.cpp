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
Signal::Signal(float frequency, float amplitude, float offset)
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

// add Signal to modulate 
// destination = (0 : frequency, 1 : amplitude, 2 : phase offset)
// signal = pointer to signal to do the modulating
// amount = amount of modulation (float between 0 - 1)
void Signal::addMod(int destination, Signal *signal, float amount)
{
    // check destination
    if(destination < 0 || destination > 2)
    {
        throw std::invalid_argument("destination must be a value between 0 and 2 (0 : frequency, 1 : amplitude, 2 : phase offset)\n");
    }
    // check amount
    if(amount < 0 || amount > 1)
    {
        throw std::invalid_argument("amount must be a value between 0 and 1\n");
    }

    // set modulator
    modulator mod;
    mod.signal = signal;
    mod.amount = amount;

    // add modulator to vertor of modulators
    mods[destination].push_back(mod);
}

// get valarray of frequencies post modulation
std::valarray<double> Signal::getFreq(std::valarray<double> ts)
{
    std::valarray<double> freqs(ts.size());
    std::fill(begin(freqs), end(freqs), freq);

    if(mods[0].size() == 0)
    {
        return freqs;
    }

    std::valarray<double> modBuffer(ts.size());
    std::fill(begin(modBuffer), end(modBuffer), freqToSemi(freq));

    for(modulator mod : mods[0])
    {
        modBuffer += mod.signal->evaluate(ts) * (mod.amount * 12);
    }

    modBuffer /= mods[0].size();
    freqs = modBuffer.apply([](double n)->double
                    {
                        return clamp(semiToFreq(n), 20, 20000);
                    }); 
    return freqs;
}

// get valarray of amplitudes, post modulation
std::valarray<double> Signal::getAmp(std::valarray<double> ts)
{
    std::valarray<double> amps(ts.size());
    std::fill(begin(amps), end(amps), amp);

    if(mods[1].size() == 0)
    {
        return amps;
    }

    // loop over the modulators
    for(modulator mod : mods[1])
    {
        // evaluate them with ts then multiply by amount
        amps += mod.signal->evaluate(ts) * mod.amount;
    }

    amps /= mods[1].size();
    amps = amps.apply([](double n)->double
            {
                return clamp(n, 0, 1);
            });
    
    return amps;
}

// get valarray of phase offsets, post modulation
std::valarray<double> Signal::getPhase(std::valarray<double> ts)
{
    std::valarray<double> phases(ts.size());
    std::fill(begin(phases), end(phases), off);

    if(mods[2].size() == 0)
    {
        return phases;
    }

    // loop over modulators
    for(modulator mod : mods[2])
    {
        phases += mod.signal->evaluate(ts) * (mod.amount * PI2);
    }

    phases /= mods[2].size();
    phases = phases.apply([](double n)->double
                {
                    return clamp(n, 0, PI2);
                });

    return phases;
}

// evaluate funcion for a Sine Signal
std::valarray<double> Sine::evaluate(std::valarray<double> ts)
{
    std::valarray<double> phases = PI2 * getFreq(ts) * ts + getPhase(ts);
    return getAmp(ts) * cos(phases);
}

// evaluate function for Triangle Signal
std::valarray<double> Triangle::evaluate(std::valarray<double> ts)
{
    std::valarray<double> cycles = getFreq(ts) * ts + getPhase(ts) / PI2;
    std::valarray<double> frac = cycles.apply(fractional);
    std::valarray<double> ys = abs(frac - 0.5);
    return normalize(unbias(ys), 1) * getAmp(ts);
}

// evaluate function for Saw Signal
std::valarray<double> Saw::evaluate(std::valarray<double> ts)
{
    std::valarray<double> cycles = getFreq(ts) * ts + getPhase(ts) / PI2;
    std::valarray<double> frac = cycles.apply(fractional);
    return normalize(unbias(frac), 1) * getAmp(ts); 
}

// evaluate function for Square Signal
std::valarray<double> Square::evaluate(std::valarray<double> ts)
{
    std::valarray<double> cycles = getFreq(ts) * ts + getPhase(ts) / PI2;
    std::valarray<double> frac = cycles.apply(fractional);
    return getAmp(ts) * unbias(frac).apply(sign);
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

// clamps value between min and max
double clamp(double n, double min, double max)
{
    return std::max(std::min(n, max), min);
}

// get frequency from number of semitones offset from C0
double semiToFreq(double semi)
{
    return A4_TUNING * std::pow(std::pow(2.0, 1.0/12.0), (semi - A4_INDEX));
}

// get number of semitones from frequency
double freqToSemi(double freq)
{
    return A4_INDEX + (std::log(freq / A4_TUNING) / std::log(std::pow(2.0, 1.0/12.0))); 
}