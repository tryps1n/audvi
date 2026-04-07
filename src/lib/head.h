#include <fftw3.h>
#include <iostream>
#include <vector>
#include <raylib.h>
#include "dr_wav.h"
#include <string.h>
#include <portaudio.h>

#define FRAMES_PER_BUFFER 256
#define SAMPLE_RATE 44100
#define NUM_CHANNELS 1
#define SAMPLE_TYPE paFloat32


struct Visualizer
{
    int WIDTH, HEIGHT, FFT_SIZE, BIN_SIZE, BAR_PX_WIDTH;
    char visMode;
    double* fft_input;
    fftw_complex* fft_output;
    fftw_plan plan;
    std::vector<float> magnitudes;
    std::vector<float> decibels;
    
    //------------------adjustments---------------------------
    float bass_weight = 0.7;
    float treble_weight = 1;
    int maxMag = 90; 
    int maxDB = 10; int minDB = -30;
    int offset = 50;
    //--------------------------------------------------------

    Visualizer(int w, int h, int fft_size);

    void Exec_FFT(int currentPos, float* monoSamples);

    void drawSpec(int i);
    void drawBar(int i);

    ~Visualizer(){
        fftw_destroy_plan(plan);
        fftw_free(fft_input);
        fftw_free(fft_output);
    }
};

struct AudioLoader
{
    float* samples;
    int totalframes, samplerate, channels;
    float* mono;
    drwav wav;
    Music music;

    bool load(const char* path);

    ~AudioLoader()
    {
        free(samples); free(mono);
        UnloadMusicStream(music); 
        drwav_uninit(&wav);
    }
};

struct MicrophoneInput 
{
    int frameIndex;
    float* circ_buffer;

    int writePos = 0; 
    int readPos = 0; 
    int bufferSize;
    int available = 0;

    float readBuffer[256];

    bool initialize(MicrophoneInput *data);
    bool checkForNewSamples(MicrophoneInput *data, int size, float* readBuffer);

    PaStream* stream = nullptr;

    ~MicrophoneInput(){
        free(circ_buffer);
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        Pa_Terminate();
    }
};