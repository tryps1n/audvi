# audvi - FFT based audio visualiser made in C++

## How to run 

cd to `/src` and run the command  
 
`g++ main.cpp vis.cpp -o ../bin/main -lraylib -lm -lpthread -ldl -lfftw3`

cd to `/bin/` and run `./main/`

### Flags

`-b` -> toggle bar display  
`-l` -> toggle line display

## Dependencies

1. [raylib](https://www.raylib.com/) used for rendering graphics and audio playback  
2. [fftw](https://www.fftw.org/) used for computing FFTs  
3. [dr_wav](https://github.com/mackron/dr_libs/blob/master/dr_wav.h) used for decoding .wav files (included in the /lib/ directory)