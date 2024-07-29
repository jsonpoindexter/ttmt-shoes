## Project Setup

### Required Files

Create a file named `secrets.h` in the root directory of the project with the following content:

```cpp
#ifndef SECRETS_H
#define SECRETS_H

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

#endif // SECRETS_H


### DY-SV8F Audio Module
This project uses the 5v DY-SV8F audio module for audio playback. The module supports both MP3 and WAV (16-bit) file formats. The supported sampling frequencies (in KHz) are: 8, 11.025, 12, 16, 22.05, 24, 32, 44.1, and 48.