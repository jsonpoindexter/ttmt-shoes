#include "Logger.h"
#include <Arduino.h>
#include <stdarg.h>

Logger::Logger(const char *serverIp, uint16_t serverPort)
        : serverIp(serverIp), serverPort(serverPort) {}

void Logger::setup() {
    // Attempt to connect to the server
    while (!client.connect(serverIp, serverPort)) {
        delay(1000);
        Serial.println("Connecting to logging server...");
    }
    Serial.println("Connected to logging server");
}

void Logger::handleClient() {
    if (!client.connected()) {
        // Try to reconnect if the connection is lost
        while (!client.connect(serverIp, serverPort)) {
            delay(1000);
            Serial.println("Reconnecting to logging server...");
        }
        Serial.println("Reconnected to logging server");
    }
}

void Logger::printf(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    String message(buffer);

    // Log to TCP client and Serial
    logToClient(message);
    Serial.print(message);
}

void Logger::print(const String &message) {
    // Log to TCP client and Serial
    logToClient(message);
    Serial.print(message);
}

void Logger::println(const String &message) {
    // Log to TCP client and Serial
    logToClient(message + "\n");
    Serial.println(message);
}

void Logger::logToClient(const String &message) {
    if (client.connected()) {
        client.print(message);
    }
}
