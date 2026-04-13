#include <cstring>
#include <iostream>
#include <raylib.h>
#define DR_WAV_IMPLEMENTATION
#include "./lib/head.h"
#include <string.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "usage: " << std::endl;
        std::cout << "./main <input-flags>" << std::endl;
        std::cout << "flags:" << std::endl;
        std::cout << "      -m -> microphone input" << std::endl;
        std::cout << "      -f -> file input" << std::endl;
        return 1;
    }
    
    MicrophoneInput *mic = nullptr;
    AudioLoader *audio = nullptr; 
    bool useMic = false; bool useBar = true;

    if (!strcmp(argv[1], "-m")) useMic = true;
    else if (!strcmp(argv[1], "-f")) useMic = false;
    else 
    {
        std::cout << "Error, invalid input flag. " << std::endl;
        std::cout << "usage: " << std::endl;
        std::cout << "./main <input-flags>" << std::endl;
        std::cout << "flags:" << std::endl;
        std::cout << "      -m -> microphone input" << std::endl;
        std::cout << "      -f -> file input" << std::endl;
        return 1;
    }

    const int WIDTH = 1024, HEIGHT = 800, FFT_SIZE = 512;
    InitWindow(WIDTH, HEIGHT, "audvi");
    InitAudioDevice();
    SetTargetFPS(60);
    
    Visualizer vis(WIDTH, HEIGHT, FFT_SIZE);
    
    if (useMic) // initialise microphone
    {
        mic = new MicrophoneInput;
        mic->initialize(mic);
    }
    else 
    {
        audio = new AudioLoader;
        // const char* file_path = audio->getPath();
        audio->file_path = audio->getPath(); 
        // const char* const_path = audio->file_path;
        audio->load(audio->file_path, vis.FFT_SIZE);
        PlayMusicStream(audio->music);
    }
    
    while(!WindowShouldClose())
    {
        if (!useMic)
        {
            audio->playUpdate();
            if(!audio->getNewSamples()) break;
            vis.Exec_FFT(audio->sampleBuffer);
        }
        else 
        {
            if(mic->checkForNewSamples(mic, FRAMES_PER_BUFFER, mic->readBuffer))
            {
                vis.Exec_FFT(mic->readBuffer);
            }
        }

        if (IsKeyPressed(KEY_SPACE)) useBar = !useBar;

        BeginDrawing();
            ClearBackground(BLACK);
            for (int i=0; i<vis.BIN_SIZE; ++i)
            {
                Color col = vis.getHSL(i);
                if (useBar) vis.drawBar(i, col);
                else if (i < vis.BIN_SIZE - 1) vis.drawSpec(i, col);
            }
        EndDrawing();

    }
    return 0;
}