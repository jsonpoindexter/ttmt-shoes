## Project Setup

### Required Files

Create a file named `secrets.h` in the root directory of the project with the following content:

```cpp
#ifndef SECRETS_H
#define SECRETS_H

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

#endif // SECRETS_H