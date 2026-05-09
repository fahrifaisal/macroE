#include <iostream>
#include <windows.h>
#include <random>
#include <thread>
#include <atomic>

// Menggunakan atomic untuk thread safety
std::atomic<bool> isRunning(true);
std::atomic<bool> isToggled(false);

void MacroThread() {
    // Setup Random Number Generator untuk delay 200-300ms
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delayDist(200, 300);

    bool wIsHeld = false;

    while (isRunning) {
        if (isToggled) {
            // Jika toggle ON dan W belum ditahan, tekan dan tahan W
            if (!wIsHeld) {
                keybd_event(0x57, 0, 0, 0); // 0x57 adalah VK code untuk 'W' (Key Down)
                wIsHeld = true;
            }
            
            // Spam E (0x45 adalah VK code untuk 'E')
            keybd_event(0x45, 0, 0, 0); // E Down
            Sleep(20);                  // Delay singkat agar game/sistem mencatat input
            keybd_event(0x45, 0, KEYEVENTF_KEYUP, 0); // E Up

            // Random delay antara 200ms - 300ms
            int randomDelay = delayDist(gen);
            Sleep(randomDelay);

        } else {
            // Jika toggle OFF, tapi W masih tertahan, lepaskan W
            if (wIsHeld) {
                keybd_event(0x57, 0, KEYEVENTF_KEYUP, 0); // W Up
                wIsHeld = false;
            }
            // Sleep sebentar saat idle untuk menghemat pemakaian CPU
            Sleep(50);
        }
    }
    
    // Safety: Pastikan tombol W dilepas saat program ditutup menggunakan 9
    if (wIsHeld) {
        keybd_event(0x57, 0, KEYEVENTF_KEYUP, 0);
    }
}

int main() {
    std::cout << "=== Macro Script Berjalan ===" << std::endl;
    std::cout << "[0] - Toggle ON/OFF" << std::endl;
    std::cout << "[9] - Exit Program" << std::endl;
    std::cout << "=============================" << std::endl;

    // Memulai thread untuk makro
    std::thread macro(MacroThread);

    bool zeroWasPressed = false;

    while (isRunning) {
        // Cek tombol 9 (0x39) untuk keluar
        if (GetAsyncKeyState(0x39) & 0x8000) {
            std::cout << "Menutup program..." << std::endl;
            isRunning = false;
            break;
        }

        // Cek tombol 0 (0x30) untuk toggle
        if (GetAsyncKeyState(0x30) & 0x8000) {
            // Memastikan toggle hanya berubah sekali per satu kali tekan (debounce)
            if (!zeroWasPressed) {
                isToggled = !isToggled;
                std::cout << "Status: " << (isToggled ? "ON (Holding W, Spamming E)" : "OFF") << std::endl;
                zeroWasPressed = true;
            }
        } else {
            zeroWasPressed = false;
        }

        // Sleep singkat untuk menghemat CPU pada loop utama
        Sleep(10);
    }

    // Tunggu thread makro selesai sebelum menutup aplikasi
    if (macro.joinable()) {
        macro.join();
    }

    return 0;
}
