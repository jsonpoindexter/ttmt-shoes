#ifndef LOGGER_H
#define LOGGER_H

#include <WiFi.h>
#include <WiFiClient.h>

class Logger {
public:
    Logger(const char *serverIp, uint16_t serverPort);

    void setup();

    void handleClient();

    // Support for Serial-like functions
    void printf(const char *format, ...);

    void print(const String &message);

    void println(const String &message);

private:
    void logToClient(const String &message);

    const char *serverIp;
    uint16_t serverPort;
    WiFiClient client;
};

#endif // LOGGER_H
