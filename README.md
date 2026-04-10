# FFT based audio visualiser

## How to run 

```
cmake .. 
make
```

Go to `/build/bin/` and run `audvi`

### Usage

```
./main <flag>
```

only `.wav` playback possible for now.

### Flags

`-m` -> process FFT through microphone input
`-f` -> process from `.wav` file

Press `<SPACE>` while playing to toggle between bar dislpay and graph display.

## Dependencies

1. [raylib](https://www.raylib.com/) used for rendering graphics and audio playback  
2. [fftw](https://www.fftw.org/) used for computing FFTs  
3. [dr_wav](https://github.com/mackron/dr_libs/blob/master/dr_wav.h) used for decoding .wav files (included in the /lib/ directory)
4. [portaudio](https://www.portaudio.com/) used for processing microphone input
5. [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) used for processing file input