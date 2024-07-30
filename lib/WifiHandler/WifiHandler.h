#ifndef WIFIHANDLER_H
#define WIFIHANDLER_H

#include <WiFi.h>

class WiFiHandler {
public:
    WiFiHandler(const char *ssid, const char *password, const char *baseHostname);

    void connect();

    static bool isConnected();


private:
    const char *ssid;
    const char *password;
    const char *baseHostname;

    static std::string formatUniqueMacAddress(uint64_t mac);

    String createUniqueHostname(const char *baseHostname);
};

#endif // WIFIHANDLER_H
