#include <cstdlib>
#include <fftw3.h>
#include <raylib.h>
#include "./lib/dr_wav.h"
#include "./lib/head.h"
#include <cmath>

//-----------------------------------------------Visualizer------------------------------------------------

// definition of visualizer constructor
Visualizer::Visualizer(int w, int h, int fft_size)
    : WIDTH(w), HEIGHT(h), FFT_SIZE(fft_size), BIN_SIZE(fft_size/2), BAR_PX_WIDTH(w / (fft_size / 2))
{
    magnitudes.resize(BIN_SIZE);
    decibels.resize(BIN_SIZE);

    fft_input = (double*) fftw_malloc(sizeof(double) * FFT_SIZE);
    fft_output = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    plan = fftw_plan_dft_r2c_1d(FFT_SIZE, fft_input, fft_output, FFTW_ESTIMATE);
}

void Visualizer::Exec_FFT(int currentPos, float* monoSamples)
{  
    // fill input array and execute fftw plan
    for (int i=0; i<FFT_SIZE; ++i) 
    {
        fft_input[i] = monoSamples[currentPos+i];
    }
    fftw_execute(plan);

    // calculate magnitudes using weights
    for (int i=0; i<BIN_SIZE; ++i)
    {
        if (i<BIN_SIZE/4)
        {
            magnitudes[i] = bass_weight * sqrt(fft_output[i][0]*fft_output[i][0] + 
                            fft_output[i][1]*fft_output[i][1]);
        }
        else if (i > 3 * BIN_SIZE / 4)
        {
            magnitudes[i] = treble_weight * sqrt(fft_output[i][0]*fft_output[i][0] + 
                         fft_output[i][1]*fft_output[i][1]); 
        }
        else
        {
            magnitudes[i] = sqrt(fft_output[i][0]*fft_output[i][0] + 
                            fft_output[i][1]*fft_output[i][1]);
        }
    }
    // convert magnitudes to decibels
    for(int i=0; i<BIN_SIZE; ++i)
    {
        decibels[i] = 10 * log10(magnitudes[i] / maxMag);
    } 
}

void Visualizer::drawBar(int i)
{
    float ratio = (decibels[i] - minDB) / (maxDB - minDB);
    if(ratio < 0) ratio = 0;

    float barHeight = ratio * (HEIGHT-offset);
    DrawRectangle(BAR_PX_WIDTH*i, HEIGHT-barHeight, BAR_PX_WIDTH, barHeight, WHITE);
}

void Visualizer::drawSpec(int i)
{
    float ratio1 = (decibels[i] - minDB) / (maxDB - minDB);
    float ratio2 = (decibels[i+1] - minDB) / (maxDB - minDB);
    if(ratio1 < 0) ratio1 = 0;
    if(ratio2 < 0) ratio2 = 0;

    float barHeight1 = ratio1 * (HEIGHT-50);
    float barHeight2 = ratio2 * (HEIGHT-50);
    
    DrawLine((BAR_PX_WIDTH*i), HEIGHT-barHeight1, (BAR_PX_WIDTH*(i+1)), HEIGHT-barHeight2,WHITE); 
}
//-----------------------------------------------AudioLoader-------------------------------------

// load function
bool AudioLoader::load(const char* path)
{
    music = LoadMusicStream(path);

    if (!drwav_init_file(&wav, path, NULL)) {
        return false;
    }
    totalframes = wav.totalPCMFrameCount;
    channels = wav.channels;
    samplerate = wav.sampleRate;

    samples = (float*) malloc(totalframes*channels*sizeof(float));
    drwav_read_pcm_frames_f32(&wav, totalframes, samples);
    
    mono = (float*) malloc(totalframes*sizeof(float));
    if (channels == 2)
    {
        for(int i=0; i<totalframes; ++i)
        {
            mono[i] = (samples[i*2] + samples[2*i+1]) / 2.0f;
        } // average odd and even samples if stereo audio
    }
    else
    {
        for (int i=0; i<totalframes; ++i)
        {
            mono[i] = samples[i];
        }
    }

    return true;
}