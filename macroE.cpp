#include <iostream>
#include <windows.h>
#include <random>
#include <thread>
#include <atomic>

std::atomic<bool> isRunning(true);
std::atomic<bool> isToggled(false);

// Fungsi untuk mengirim input level Hardware (Scan Code) untuk DirectInput Games
void SendKeyByScanCode(WORD scanCode, bool isKeyDown) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = scanCode;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wVk = 0; // Wajib 0 agar sistem membaca Scan Code, bukan Virtual Key

    if (isKeyDown) {
        input.ki.dwFlags = KEYEVENTF_SCANCODE; // Key Press
    } else {
        input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP; // Key Release
    }
    SendInput(1, &input, sizeof(INPUT));
}

void MacroThread() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delayDist(200, 300);

    bool wIsHeld = false;

    // Scan Codes
    const WORD SCANCODE_W = 0x11;
    const WORD SCANCODE_E = 0x12;

    while (isRunning) {
        if (isToggled) {
            // Tahan W
            if (!wIsHeld) {
                SendKeyByScanCode(SCANCODE_W, true);
                wIsHeld = true;
            }
            
            // Spam E
            SendKeyByScanCode(SCANCODE_E, true);   // E Down
            Sleep(25);                             // Delay tekan agak lama agar FiveM sempat mendeteksi
            SendKeyByScanCode(SCANCODE_E, false);  // E Up

            // Random delay
            int randomDelay = delayDist(gen);
            Sleep(randomDelay);

        } else {
            // Lepas W jika toggle OFF
            if (wIsHeld) {
                SendKeyByScanCode(SCANCODE_W, false);
                wIsHeld = false;
            }
            Sleep(50);
        }
    }
    
    // Safety release sebelum exit
    if (wIsHeld) {
        SendKeyByScanCode(SCANCODE_W, false);
    }
}

int main() {
    std::cout << "=== Macro Script GTA V / FiveM ===" << std::endl;
    std::cout << "[0] - Toggle ON/OFF" << std::endl;
    std::cout << "[9] - Exit Program" << std::endl;
    std::cout << "==================================" << std::endl;

    std::thread macro(MacroThread);
    bool zeroWasPressed = false;

    while (isRunning) {
        // Cek tombol 9 (Exit)
        if (GetAsyncKeyState(0x39) & 0x8000) {
            std::cout << "Menutup program..." << std::endl;
            isRunning = false;
            break;
        }

        // Cek tombol 0 (Toggle)
        if (GetAsyncKeyState(0x30) & 0x8000) {
            if (!zeroWasPressed) {
                isToggled = !isToggled;
                std::cout << "Status: " << (isToggled ? "ON (Maju & Spam E)" : "OFF") << std::endl;
                zeroWasPressed = true;
            }
        } else {
            zeroWasPressed = false;
        }

        Sleep(10);
    }

    if (macro.joinable()) {
        macro.join();
    }

    return 0;
}
