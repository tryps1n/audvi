#include <cstring>
#include <iostream>
#include <raylib.h>
#define DR_WAV_IMPLEMENTATION
#include "./lib/head.h"
#include <cmath>
#include <string.h>

float getVolume(float* buffer, int size)
    {
        float sum = 0;
        for (int i=0; i<size; ++i)
        {
            sum += buffer[i] * buffer[i];
        }
        return sqrt(sum/size);
    }

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
    
    MicrophoneInput mic;
    mic.initialize(&mic);
    
    const int WIDTH = 800, HEIGHT = 600, FFT_SIZE = 256;

    // initialise raylib
    InitWindow(WIDTH, HEIGHT, "audvi");
    // InitAudioDevice();
    SetTargetFPS(60);

    Visualizer vis(800, 600, FFT_SIZE);

    // load audio from provided path
    // dont access contents of audio before calling load()
    // AudioLoader audio;
    // if(!audio.load(path)){
    //     std::cout << "error loading file" << std::endl;
    //     return 0;
    // }

    // float lastTime = GetTime(); float currentTime = 0;
    // long long currentPos;

    // PlayMusicStream(audio.music);

    
    while(!WindowShouldClose())
    {
        // UpdateMusicStream(audio.music);

        // track time
        // float nowTime = GetTime();
        // float deltaTime = nowTime - lastTime;
        // lastTime = nowTime;

        // if (IsMusicStreamPlaying(audio.music))
        // {
        //     currentTime += deltaTime;
        // }
        // currentPos = (long long) (currentTime * audio.samplerate);

        if (!mic.checkForNewSamples(&mic, FRAMES_PER_BUFFER, mic.readBuffer))
        {
            // std::cout << "hi" << std::endl;
            continue;
        }

        vis.Exec_FFT(0, mic.readBuffer);
        std::cout << getVolume(mic.readBuffer, 256) << std::endl;

        BeginDrawing();
            ClearBackground(BLACK);
            for (int i=0; i<vis.BIN_SIZE; ++i)
            {
                if (!strcmp(argv[1], "-b")) vis.drawBar(i);
                else if(i<vis.BIN_SIZE-1) vis.drawSpec(i);
            }
        EndDrawing();

    }
    return 0;
}