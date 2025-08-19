# pragma once

#include <valarray>
#include <map>

const double PI2 = M_PI * 2;

const int A4_TUNING = 440; // frequency (Hz) of A4
const float A4_INDEX = 57; // number of semitones from C0 to A4

class Signal; // Signal class prototype

// Signal and amount to modulate another Signal
typedef struct modulator
{
    Signal *signal; // pointer to modulation signal
    float amount; // amount of modulation
} modulator;

// wav file header (for writing files not reading)
typedef struct wavHeader 
{
    // header chunk
    uint32_t fileId = 'FFIR';
    uint32_t fileSize;
    uint32_t formatId = 'EVAW';
    // fmt chunk
    uint32_t fmtId = ' tmf';
    uint32_t fmtSize = 16;
    uint16_t audioFmt = 1;
    uint16_t numChannels = 1;
    uint32_t sampleRate;
    uint32_t bytesPerSec;
    uint16_t blockSize = 2;
    uint16_t bitsPerSample = 16;
    // data chunk
    uint32_t dataId = 'atad';
    uint32_t dataSize;
} __attribute__((__packed__))
wavHeader;

/* Wave class */
class Wave
{
    public:
    std::valarray<double> ys; // samples
    int rate; // sample rate of the wave
    float dur; // duration of the wave

    Wave(std::valarray<double> samples, int sampleRate, float duration);

    void toWav(std::string filename);
};

/* Signal class */
class Signal
{
    public:
    float freq; // frequency of the signal
    float amp; // amplitude of the signal
    float off; // phase offset of the signal (in radians)
    
    Signal(float frequency = 440, float amplitude = 1.0, float offset = 0);
    Wave makeWave(int sampleRate, float duration);
    void addMod(int destination, Signal *signal, float amount);
    
    protected:
    std::vector<modulator> mods[3];
    
    virtual std::valarray<double> evaluate(std::valarray<double> ts);
    std::valarray<double> getFreq(std::valarray<double> ts);
    std::valarray<double> getAmp(std::valarray<double> ts);
    std::valarray<double> getPhase(std::valarray<double> ts);
};

/* Sine Signal */
class Sine : public Signal
{
    public:
    Sine(float frequency, float amplitude, float offset) : Signal(frequency, amplitude, offset){}

    protected:
    virtual std::valarray<double> evaluate(std::valarray<double> ts);

};

/* Triangle Signal */
class Triangle : public Signal
{
    public:
    Triangle(float frequency, float amplitude, float offset) : Signal(frequency, amplitude, offset){}

    protected:
    virtual std::valarray<double> evaluate(std::valarray<double> ts);
};

/* Saw Signal */
class Saw : public Signal
{
    public:
    Saw(float frequency, float amplitude, float offset) : Signal(frequency, amplitude, offset){}

    protected:
    virtual std::valarray<double> evaluate(std::valarray<double> ts);
};

/* Square Signal */
class Square : public Signal
{
    public:
    Square(float frequency, float amplitude, float offset) : Signal(frequency, amplitude, offset){}

    protected:
    virtual std::valarray<double> evaluate(std::valarray<double> ts);
};

/* Helper Maths Functions */
std::valarray<double> unbias(std::valarray<double> ys);
std::valarray<double> normalize(std::valarray<double> ys, float amp);
double fractional(double n);
double sign(double n);
double clamp(double n, double min, double max);
double semiToFreq(double semis);
double freqToSemi(double freq);