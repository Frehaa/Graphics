class AudioWrapper;
typedef int16_t(*sample_fn)(AudioWrapper*);

class AudioWrapper {
public:
    int samplesPerSecond;
    uint32_t runningSampleIndex;
    int bufferSeconds;
    int bytesPerSample = sizeof(int16_t) * 2;
    int bufferSize;
    sample_fn sampler;
    LPDIRECTSOUNDBUFFER buffer;

    AudioWrapper(HWND windowHandle, int samplesPerSecond, int bufferSeconds, sample_fn sampler) : 
                    samplesPerSecond(samplesPerSecond), bufferSeconds(bufferSeconds), sampler(sampler) {
        bufferSize = samplesPerSecond * bytesPerSample * bufferSeconds;
        runningSampleIndex = 0;
        initDirectSound(windowHandle);

        tick(0);
        buffer->Play(0, 0, DSBPLAY_LOOPING);
    }

    void tick(int t) {
        DWORD playCursor;
        DWORD writeCursor;
        if (FAILED(buffer->GetCurrentPosition(&playCursor, &writeCursor))) {
            println("Failed to get sound cursor positions");
            return;
        }

        DWORD bytesToLock = (runningSampleIndex * bytesPerSample) % bufferSize;
        DWORD bytesToWrite = calculateBytesToWrite(bytesToLock, playCursor); 

        void *region1;
        DWORD region1Size;
        void *region2;
        DWORD region2Size;
        if (FAILED(buffer->Lock(bytesToLock, bytesToWrite, &region1, &region1Size, &region2, &region2Size, 0))) {
            println("Failed to lock sound buffer");
            return;
        }

        writeSamples(region1, region1Size);
        writeSamples(region2, region2Size);

        buffer->Unlock(region1, region1Size, region2, region2Size);
    }

private:
    void initDirectSound(HWND windowHandle) {
        // TODO: Do we need to dynamically load this? 
        // * Not if we include Dsound.lib in compilation
        HMODULE lib = LoadLibraryA("dsound.dll");
        if (lib == nullptr) {
            println("Failed to load library");
            return;
        }
        direct_sound_create *DirectSoundCreate = (direct_sound_create*) GetProcAddress(lib, "DirectSoundCreate");

        LPDIRECTSOUND directSound;
        if (//directSoundCreate == nullptr || 
            FAILED(DirectSoundCreate(0, &directSound, 0))) {
            println("Failed to load library");
            return;
        }

        if (FAILED(directSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY))) {
            println("Failed to set cooperative level");
            return;
        }

        DSBUFFERDESC setupBufferDescription = {0};
        setupBufferDescription.dwSize = sizeof(setupBufferDescription);
        setupBufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

        // Setup buffer is what the WINAPI refers to as the primary buffer
        LPDIRECTSOUNDBUFFER setupSoundBuffer;
        if (FAILED(directSound->CreateSoundBuffer(&setupBufferDescription, &setupSoundBuffer, 0))) {
            println("Failed to create setup sound buffer");
            return;
        }

        WAVEFORMATEX waveFormat = initWaveFormat(samplesPerSecond);
        if (FAILED(setupSoundBuffer->SetFormat(&waveFormat))) {
            println("Failed to set sound buffer format");
            return;
        }

        DSBUFFERDESC bufferDescription = {0};
        bufferDescription.dwSize = sizeof(bufferDescription);
        bufferDescription.dwFlags = 0;
        bufferDescription.dwBufferBytes = bufferSize;
        bufferDescription.lpwfxFormat = &waveFormat;
        
        LPDIRECTSOUNDBUFFER soundBuffer;
        if (FAILED(directSound->CreateSoundBuffer(&bufferDescription, &soundBuffer, 0))) {
            println("Failed to create sound buffer");
            return;
        }
        this->buffer = soundBuffer;
    }
    
    WAVEFORMATEX initWaveFormat(int samplesPerSecond){
        WAVEFORMATEX waveFormat = {};
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = 2;
        waveFormat.nSamplesPerSec = samplesPerSecond;
        waveFormat.wBitsPerSample = 16;
        waveFormat.cbSize = 8;
        waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        return waveFormat;
    }

    void writeSamples(void *region, int regionSize) {
        int16_t *sampleOut = (int16_t *)region;
        int sampleCount = regionSize / bytesPerSample;
        for (int i = 0; i < sampleCount; ++i) {
            int16_t sampleValue = sample();
            // Write and increment to pointers
            *sampleOut++ = sampleValue; 
            *sampleOut++ = sampleValue;
        }
    }

    int16_t sample() {
        int16_t sampleValue = sampler(this);
        runningSampleIndex += 1;
        return sampleValue;
    }

    int calculateBytesToWrite(DWORD bytesToLock, DWORD playCursor) {
        if (bytesToLock > playCursor) {
            return playCursor + bufferSize - bytesToLock;
        } else {
            return playCursor - bytesToLock;
        }
    }
};

int16_t squareSoundSampler(AudioWrapper *audioWrapper) {
    int toneHz = 256;
    int squareWavePeriod = audioWrapper->samplesPerSecond / toneHz;
    int halfSquareWavePeriod = squareWavePeriod / 2;
    int16_t upTone = 1000;
    int16_t downTone = -1000;

    return ((audioWrapper->runningSampleIndex / halfSquareWavePeriod) % 2)? upTone : downTone;
}

int16_t sineSoundSampler(AudioWrapper *audioWrapper) {
    float_t toneHz = 512;
    float_t wavePeriod = (float) audioWrapper->samplesPerSecond / toneHz;
    float_t t = audioWrapper->runningSampleIndex / wavePeriod;
    float_t sineValue = sinf(t);
    int16_t toneVolume = 3500;
    return (int16_t)(sineValue * toneVolume);
}