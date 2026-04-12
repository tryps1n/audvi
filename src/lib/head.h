#include <fftw3.h>
#include <iostream>
#include <vector>
#include <raylib.h>
#include "dr_wav.h"
#include <string.h>
#include <portaudio.h>
#include "./tinyfiledialogs/tinyfiledialogs.h"

#define FRAMES_PER_BUFFER 512
#define SAMPLE_RATE 44100
#define NUM_CHANNELS 1
#define SAMPLE_TYPE paFloat32
#define HSL_SAT 60.0f
#define HSL_LIGHT 30.0f

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
    float deep_bass_weight = 0.08;
    float bass_weight = 0.4;
    float treble_weight = 1;
    float normal_weight = 0.8;
    int maxMag = BIN_SIZE; 
    int maxDB = -20; int minDB = -80;
    int offset = 50;
    //--------------------------------------------------------

    Visualizer(int w, int h, int fft_size);

    void Exec_FFT(float* monoSamples);

    void drawSpec(int i, Color col);
    void drawBar(int i, Color col);
    Color getHSL(int j);

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
    float lastTime = 0;
    float playTime = 0;
    long currentPos = 0;
    long long framePos = 0;
    float* sampleBuffer;
    int FFT_SIZE; int hopSize;

    drwav wav;
    Music music;
    const char* file_path;

    bool load(const char* path, int size);
    const char* getPath();
    void playUpdate();
    bool getNewSamples();

    ~AudioLoader()
    {
        free(samples); free(mono);
        UnloadMusicStream(music); 
        drwav_uninit(&wav);
        free(sampleBuffer);
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

    float readBuffer[FRAMES_PER_BUFFER];

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