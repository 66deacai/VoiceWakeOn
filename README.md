# VoiceWakeOn

### Description
This project implements a lightweight keyword spotting(KWS) system running on RK3568(or similar rockchip system). 
The project is based on C/C++.

### Overview
The system includes:

- Real-time steaming audio pipeline by ALSA
- Voice Activity Detection
- Log-Mel feature extraction
- CMVN normalization
- RKNN model inference
- Image playing by DRM

### Sample
<video src="https://github.com/66deacai/VoiceWakeOn/raw/refs/heads/main/docs/rk3568_demo.mp4" width="100%" controls></video>

### workflow

```mermaid
flowchart TD

    Start[Start up] --> |display static image by DRM interface| Static(static image)
    subgraph ALSA_THREAD[Audio capture thread]
    ALSA(ALSA Capture)
    end

    subgraph RingBuffer[ringbuffer]
    Buffer(Ring Buffer)
    end

    subgraph VOICE_DETECTION_THREAD[Voice Detection thread]
    VAD(VAD) --> |Detected voice|Trans(Log-mel + CMVN)
    Trans --> RKNN(RKNN model)
    RKNN -->  Decision(Decision)
    end
    
    Static --> ALSA
    ALSA --> Buffer(Ring buffer)
    Buffer --> VAD
    Decision --> |Key word detected|Dynamic(Dynamic image)
```

### Getting Started

- compile
```shell
./make.sh
```

- run
```shell
./build/voice_cap
```

### Dependencies
- ALSA
- RKNN Toolkit
- GCC (arm-linux-gnueabihf)
- CMake





***A practice project,  for leaning purpose only***