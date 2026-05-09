#include <iostream>
#include <windows.h>
#include <random>
#include <thread>
#include <atomic>

std::atomic<bool> isRunning(true);
std::atomic<bool> isToggled(false);

// Fungsi untuk mengirim input level Hardware (Scan Code)
void SendKeyByScanCode(WORD scanCode, bool isKeyDown) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = scanCode;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wVk = 0; // Wajib 0 agar sistem membaca Scan Code

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
    
    // Distribusi random agar terasa seperti manusia asli (Humanized Input)
    std::uniform_int_distribution<> spamDist(100, 250); // Jeda antar spam E (100-250ms)
    std::uniform_int_distribution<> holdDist(20, 60);   // Lama tombol E ditekan ke bawah (20-60ms)

    bool wIsHeld = false;

    // Scan Codes untuk game DirectInput (seperti GTA V)
    const WORD SCANCODE_W = 0x11;
    const WORD SCANCODE_E = 0x12;

    while (isRunning) {
        if (isToggled) {
            // Tahan W
            if (!wIsHeld) {
                SendKeyByScanCode(SCANCODE_W, true);
                wIsHeld = true;
            }
            
            // Spam E dengan delay natural
            SendKeyByScanCode(SCANCODE_E, true);   // Tekan E
            Sleep(holdDist(gen));                  // Tahan E sejenak (Natural tap delay)
            SendKeyByScanCode(SCANCODE_E, false);  // Lepas E

            // Delay acak sebelum menekan E lagi (100-250ms)
            Sleep(spamDist(gen));

        } else {
            // Lepas W jika toggle dimatikan
            if (wIsHeld) {
                SendKeyByScanCode(SCANCODE_W, false);
                wIsHeld = false;
            }
            Sleep(50);
        }
    }
    
    // Safety: pastikan tombol W dilepas saat exit
    if (wIsHeld) {
        SendKeyByScanCode(SCANCODE_W, false);
    }
}

int main() {
    std::cout << "=== Macro Script GTA V / FiveM ===" << std::endl;
    std::cout << "Fitur: Humanized Input (Randomized Delays)" << std::endl;
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
                std::cout << "Status: " << (isToggled ? "ON (Maju & Spam E Natural)" : "OFF") << std::endl;
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
