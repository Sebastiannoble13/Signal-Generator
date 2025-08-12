# pragma once

#include <valarray>

const double PI2 = M_PI * 2;

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
    int freq; // frequency of the signal
    float amp; // amplitude of the signal
    float off; // phase offset of the signal (in radians)
    
    Signal(int frequency, float amplitude, float offset);
    Wave makeWave(int sampleRate, float duration);
    
    protected:
    virtual std::valarray<double> evaluate(std::valarray<double> ts);
};

/* Sine Signal */
class Sine : public Signal
{
    public:
    Sine(int frequency, float amplitude, float offset) : Signal(frequency, amplitude, offset){}

    protected:
    virtual std::valarray<double> evaluate(std::valarray<double> ts);

};
