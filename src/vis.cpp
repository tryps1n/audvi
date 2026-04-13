#include <cstdlib>
#include <fftw3.h>
#include <portaudio.h>
#include <raylib.h>
#include "./lib/dr_wav.h"
#include "./lib/head.h"
#include <cmath>

//-----------------------------------------------Visualizer------------------------------------------------

// definition of visualizer constructor
Visualizer::Visualizer(int w, int h, int fft_size)
    : WIDTH(w), HEIGHT(h), FFT_SIZE(fft_size), BIN_SIZE(fft_size/2), BAR_PX_WIDTH(w / FFT_SIZE)
{
    magnitudes.resize(BIN_SIZE);
    decibels.resize(BIN_SIZE);

    fft_input = (double*) fftw_malloc(sizeof(double) * FFT_SIZE);
    fft_output = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    plan = fftw_plan_dft_r2c_1d(FFT_SIZE, fft_input, fft_output, FFTW_ESTIMATE);
}

void Visualizer::Exec_FFT(float* monoSamples)
{  
    // fill input array and execute fftw plan
    for (int i=0; i<FFT_SIZE; ++i) 
    {
        fft_input[i] = monoSamples[i];
    }
    fftw_execute(plan);

    // calculate magnitudes using weights
    for (int i=0; i<BIN_SIZE; ++i)
    {
        if (i < BIN_SIZE/8)
        {
            magnitudes[i] = deep_bass_weight * sqrt(fft_output[i][0]*fft_output[i][0] + 
                            fft_output[i][1]*fft_output[i][1]);
        }
        else if (i > BIN_SIZE/8 && i<BIN_SIZE/4)
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
            magnitudes[i] = normal_weight * sqrt(fft_output[i][0]*fft_output[i][0] + 
                            fft_output[i][1]*fft_output[i][1]);
        }
    }
    // convert magnitudes to decibels
    for(int i=0; i<BIN_SIZE; ++i)
    {
        decibels[i] = 20 * log10(magnitudes[i] / maxMag);
    } 
}

void Visualizer::drawBar(int i, Color col)
{
    float ratio = (decibels[i] - minDB) / (maxDB - minDB);
    if(ratio < 0) ratio = 0;

    float barHeight = ratio * (HEIGHT-offset);
    DrawRectangle(BAR_PX_WIDTH*i, HEIGHT-barHeight, BAR_PX_WIDTH, barHeight, col);
    DrawRectangle((BIN_SIZE*2 - 1 - i)*BAR_PX_WIDTH, HEIGHT-barHeight, BAR_PX_WIDTH, barHeight, col);
}

void Visualizer::drawSpec(int i, Color col)
{
    float ratio1 = (decibels[i] - minDB) / (maxDB - minDB);
    float ratio2 = (decibels[i+1] - minDB) / (maxDB - minDB);
    if(ratio1 < 0) ratio1 = 0;
    if(ratio2 < 0) ratio2 = 0;

    float barHeight1 = ratio1 * (HEIGHT-50);
    float barHeight2 = ratio2 * (HEIGHT-50);
    
    DrawLine((BAR_PX_WIDTH*i), HEIGHT-barHeight1, (BAR_PX_WIDTH*(i+1)), HEIGHT-barHeight2,col); 
    DrawLine((BIN_SIZE*2 - 1 - i)*BAR_PX_WIDTH, HEIGHT-barHeight1, (BIN_SIZE*2 - 1 - i - 1)*BAR_PX_WIDTH, HEIGHT-barHeight2,col); }

Color Visualizer::getHSL(int j)
{
    float hue = ((j-1)/(float) BIN_SIZE) * 360.0f;
    return ColorFromHSV(hue, 0.6, 0.6);
}
//-----------------------------------------------AudioLoader-------------------------------------

// load function
bool AudioLoader::load(const char* path, int size)
{
    music = LoadMusicStream(path);
    FFT_SIZE = size;

    if (!drwav_init_file(&wav, path, NULL)) {
        return false;
    }
    totalframes = wav.totalPCMFrameCount;
    channels = wav.channels;
    samplerate = wav.sampleRate;

    samples = new float[totalframes*channels];
    drwav_read_pcm_frames_f32(&wav, totalframes, samples);
    sampleBuffer = (float*) malloc(FFT_SIZE*sizeof(float));
    
    mono = new float[totalframes*channels];
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

const char* AudioLoader::getPath()
{
    const char* filename = tinyfd_openFileDialog(
    "Select Audio File",     // Title
    "",                      // Default path
    1,                       // Number of filters
    (char const*[]){"*.wav"},  // File patterns
    "Select .wav file",           // Description
    0                        // Allow multiple selects? (0 = single)
    );
    return filename;
}

void AudioLoader::playUpdate()
{
    UpdateMusicStream(this->music);

    float currentTime = GetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    if (IsMusicStreamPlaying(this->music)) playTime += deltaTime;
    currentPos = (this->samplerate) * playTime;
}

bool AudioLoader::getNewSamples()
{
    if (!IsMusicStreamPlaying(this->music)) return false;
    // if music not playing
    for (int i=0; i<FFT_SIZE; ++i)
    {
        if (framePos+i >= totalframes) sampleBuffer[i] = 0;
        else sampleBuffer[i] = mono[framePos+i];
    }
    int hopSize = floor(wav.sampleRate / 60.0f);
    framePos += hopSize; 
    if (framePos > totalframes) return false;
    return true;
}

// ------------------------------------------------MicrophoneInput--------------------------------

static int recordCallback(const void* inputBuffer, void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void* Data)
{
    MicrophoneInput* data = (MicrophoneInput*) Data;
    const float* input = (const float*) inputBuffer;
    // recast pointers into required types

    for (int i=0; i<framesPerBuffer; ++i)
    {
        data->circ_buffer[data->writePos] = input[i]; //copy data from buffer to circular buffer 
        
        data->writePos++; 
        if (data->writePos >= data->bufferSize) data->writePos = 0;
        // check if writePos goes out of bounds. wrap around if it does.
        if (data->available < data->bufferSize) data->available++;
    }
    return paContinue;
}

bool MicrophoneInput::initialize(MicrophoneInput *data)
{
    PaError err;

    data->frameIndex = 0; 
    data->bufferSize = FRAMES_PER_BUFFER * 2;
    data->circ_buffer = (float*) malloc(data->bufferSize*sizeof(float));

    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    err = Pa_OpenDefaultStream(&stream, NUM_CHANNELS, 0, SAMPLE_TYPE, SAMPLE_RATE, 
        FRAMES_PER_BUFFER, recordCallback, data);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    return true;
}

bool MicrophoneInput::checkForNewSamples(MicrophoneInput *data, int size, float* readBuffer)
{
    if (data->available < size) return false;
    // check if enough data available
    int idx;
    for (int i=0; i<size; ++i)
    {
        idx = data->readPos+i;
        if (idx >= data->bufferSize) idx -= data->bufferSize; // wrap around the buffer

        readBuffer[i] = data->circ_buffer[idx];
    }
    if (idx < data->bufferSize) data->readPos = idx;
    else data->readPos += idx; 
    // update readpos

    data->available -= size; // size amount of data taken away
    return true;
}