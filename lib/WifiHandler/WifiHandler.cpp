#include "WiFiHandler.h"

#include <string>
#include <sstream>
#include <iomanip>

WiFiHandler::WiFiHandler(const char *ssid, const char *password, const char *hostname)
        : ssid(ssid), password(password), baseHostname(hostname) {}

void WiFiHandler::connect() {

    WiFiClass::hostname(createUniqueHostname(baseHostname));
    WiFi.begin(ssid, password);
    while (WiFiClass::status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    // Print out IP address
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(WiFiClass::getHostname());
}

bool WiFiHandler::isConnected() {
    return WiFiClass::status() == WL_CONNECTED;
}

std::string WiFiHandler::formatUniqueMacAddress(uint64_t mac) {
    std::stringstream macStream;
    macStream << std::hex << std::setfill('0');
    for (int i = 2; i >= 0; --i) {
        macStream << std::setw(2) << ((mac >> (i * 8)) & 0xFF);
        if (i > 0) {
            macStream << ":";
        }
    }
    return macStream.str();
}

String WiFiHandler::createUniqueHostname(const char *baseHostname) {
    std::string uniqueHostnameStr = std::string(baseHostname) + "-" + formatUniqueMacAddress(ESP.getEfuseMac());
    return String(uniqueHostnameStr.c_str());
}
