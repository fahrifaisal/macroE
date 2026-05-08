#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

int main() {
    // Default delay diubah ke 100ms
    int holdDelay = 100;
    int spamDelay = 100;
    bool active = false;

    // Membaca file config.txt
    std::ifstream configFile("config.txt");
    if (configFile.is_open()) {
        std::string line;
        while (getline(configFile, line)) {
            if (line.find("HOLD_DELAY=") != std::string::npos) {
                holdDelay = std::stoi(line.substr(11));
            } else if (line.find("SPAM_DELAY=") != std::string::npos) {
                spamDelay = std::stoi(line.substr(11));
            }
        }
        configFile.close();
        std::cout << "Config loaded: Hold " << holdDelay << "ms, Spam " << spamDelay << "ms" << std::endl;
    } else {
        std::cout << "config.txt tidak ditemukan, menggunakan default (100ms)." << std::endl;
    }

    std::cout << "F4: Toggle ON/OFF | Angka 0: MATIKAN PROGRAM & EXIT" << std::endl;

    while (true) {
        // Toggle ON/OFF dengan F4
        if (GetAsyncKeyState(VK_F4) & 1) {
            active = !active;
            if (active) {
                std::cout << "Status: ON (W Hold + E Spam)" << std::endl;
                keybd_event(0x57, 0, 0, 0); // W Down
            } else {
                std::cout << "Status: OFF" << std::endl;
                keybd_event(0x57, 0, KEYEVENTF_KEYUP, 0); // W Up
                keybd_event(0x45, 0, KEYEVENTF_KEYUP, 0); // E Up
            }
        }

        // Tombol Panic/Exit menggunakan Angka 0 (0x30)
        if (GetAsyncKeyState(0x30)) {
            // Pastikan semua tombol dilepas sebelum exit
            keybd_event(0x57, 0, KEYEVENTF_KEYUP, 0);
            keybd_event(0x45, 0, KEYEVENTF_KEYUP, 0);
            std::cout << "Panic Button! Program closed." << std::endl;
            return 0;
        }

        if (active) {
            keybd_event(0x45, 0, 0, 0); // E Down
            Sleep(holdDelay);           
            keybd_event(0x45, 0, KEYEVENTF_KEYUP, 0); // E Up
            Sleep(spamDelay);           
        } else {
            Sleep(100); 
        }
    }
    return 0;
}
