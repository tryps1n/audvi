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
        std::cout << "Usage: " << std::endl;
        std::cout << "./main <input-flags>" << std::endl;
        std::cout << "Flags:" << std::endl;
        std::cout << "      -m -> microphone input" << std::endl;
        std::cout << "      -f -> file input" << std::endl;
        return 1;
    }
    
    MicrophoneInput *mic = nullptr;
    AudioLoader *audio = nullptr; 
    bool useMic = false; bool useBar = true;

    if (!strcmp(argv[1], "-m")) useMic = true;

    const int WIDTH = 800, HEIGHT = 600, FFT_SIZE = 256;
    InitWindow(WIDTH, HEIGHT, "audvi");
    InitAudioDevice();
    SetTargetFPS(60);
    
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
        audio->load(audio->file_path);
        PlayMusicStream(audio->music);
    }
    
    Visualizer vis(800, 600, FFT_SIZE);
    
    while(!WindowShouldClose())
    {
        if (!useMic)
        {
            audio->playUpdate();
            vis.Exec_FFT(audio->currentPos, audio->mono);
        }
        else 
        {
            mic->checkForNewSamples(mic, FRAMES_PER_BUFFER, mic->readBuffer);
            vis.Exec_FFT(0, mic->readBuffer);
        }

        if (IsKeyPressed(KEY_SPACE)) useBar = !useBar;

        BeginDrawing();
            ClearBackground(BLACK);
            for (int i=0; i<vis.BIN_SIZE; ++i)
            {
                if (useBar) vis.drawBar(i);
                else if (i < vis.BIN_SIZE - 1) vis.drawSpec(i);
            }
        EndDrawing();

    }
    return 0;
}