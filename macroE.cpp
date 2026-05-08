#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <random> // Library untuk angka acak

int main() {
    int holdDelay = 150;
    int spamDelay = 160;
    bool active = false;

    // Setup Random Number Generator
    std::random_device rd;
    std::mt19937 gen(rd());

    std::ifstream configFile("config.txt");
    if (configFile.is_open()) {
        std::string line;
        while (getline(configFile, line)) {
            if (line.find("HOLD_DELAY=") != std::string::npos) holdDelay = std::stoi(line.substr(11));
            else if (line.find("SPAM_DELAY=") != std::string::npos) spamDelay = std::stoi(line.substr(11));
        }
        configFile.close();
    }

    std::cout << "F4: Toggle ON/OFF | Angka 0: EXIT" << std::endl;

    while (true) {
        if (GetAsyncKeyState(VK_F4) & 1) {
            active = !active;
            if (active) {
                std::cout << "Status: ON" << std::endl;
                keybd_event(0x57, 0, 0, 0); // W Down
            } else {
                std::cout << "Status: OFF" << std::endl;
                keybd_event(0x57, 0, KEYEVENTF_KEYUP, 0); // W Up
                keybd_event(0x45, 0, KEYEVENTF_KEYUP, 0); // E Up
            }
        }

        if (GetAsyncKeyState(0x30) & 0x8000) {
            keybd_event(0x57, 0, KEYEVENTF_KEYUP, 0);
            keybd_event(0x45, 0, KEYEVENTF_KEYUP, 0);
            return 0;
        }

        if (active) {
            // Membuat variasi delay +/- 20% dari nilai asli
            std::uniform_int_distribution<> distHold(holdDelay - (holdDelay * 0.2), holdDelay + (holdDelay * 0.2));
            std::uniform_int_distribution<> distSpam(spamDelay - (spamDelay * 0.2), spamDelay + (spamDelay * 0.2));

            keybd_event(0x45, 0, 0, 0); // E Down
            Sleep(distHold(gen));       // Delay acak untuk menahan E
            keybd_event(0x45, 0, KEYEVENTF_KEYUP, 0); // E Up
            Sleep(distSpam(gen));       // Delay acak antar spam E
        } else {
            Sleep(100); 
        }
    }
    return 0;
}
