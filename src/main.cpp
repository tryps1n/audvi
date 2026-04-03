#include <cstring>
#include <iostream>
#include <raylib.h>
#define DR_WAV_IMPLEMENTATION
#include "./lib/head.h"

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << std::endl;
        std::cout << "./main <path> <flags>" << std::endl;
        std::cout << "      -b -> show bars" << std::endl;
        std::cout << "      -l -> show line graph" << std::endl;
        return 1;
    } 
    
    const char* path = argv[1];

    const int WIDTH = 800, HEIGHT = 600, FFT_SIZE = 512;

    // initialise raylib
    InitWindow(WIDTH, HEIGHT, "audvi");
    InitAudioDevice();
    SetTargetFPS(30);

    Visualizer vis(800, 600, FFT_SIZE);

    // load audio from provided path
    // dont access contents of audio before calling load()
    AudioLoader audio;
    if(!audio.load(path)){
        std::cout << "error loading file" << std::endl;
        return 0;
    }

    float lastTime = GetTime(); float currentTime = 0;
    long long currentPos;

    PlayMusicStream(audio.music);

    while(!WindowShouldClose())
    {
        UpdateMusicStream(audio.music);

        // track time
        float nowTime = GetTime();
        float deltaTime = nowTime - lastTime;
        lastTime = nowTime;

        if (IsMusicStreamPlaying(audio.music))
        {
            currentTime += deltaTime;
        }
        currentPos = (long long) (currentTime * audio.samplerate);

        vis.Exec_FFT(currentPos, audio.mono);

        BeginDrawing();
            ClearBackground(BLACK);
            for (int i=0; i<vis.BIN_SIZE; ++i)
            {
                if (!strcmp(argv[2], "-b")) vis.drawBar(i);
                else if(i<vis.BIN_SIZE-1) vis.drawSpec(i);
            }
        EndDrawing();

    }
    return 0;
}