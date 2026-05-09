#include <iostream>
#include <windows.h>
#include <random>
#include <thread>
#include <atomic>
#include <string>
#include <ctime>
#include <iomanip>

std::atomic<bool> isRunning(true);
std::atomic<bool> isToggled(false);
std::atomic<bool> isTestMode(false); 

// Konstanta Scan Codes untuk DirectInput Games
const WORD SC_E = 0x12;
const WORD SC_X = 0x2D;
const WORD SC_4 = 0x05;
const WORD SC_5 = 0x06;

// Fungsi Logging dengan Timestamp
void Log(const std::string& message) {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    std::cout << "[" << std::setfill('0') << std::setw(2) << now->tm_hour << ":" 
              << std::setfill('0') << std::setw(2) << now->tm_min << ":" 
              << std::setfill('0') << std::setw(2) << now->tm_sec << "] " 
              << message << std::endl;
}

// Mengirim input level Hardware (Scan Code)
void SendKeyByScanCode(WORD scanCode, bool isKeyDown) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = scanCode;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wVk = 0; 

    if (isKeyDown) {
        input.ki.dwFlags = KEYEVENTF_SCANCODE; 
    } else {
        input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP; 
    }
    SendInput(1, &input, sizeof(INPUT));
}

// Menekan tombol dengan durasi tahan (hold) acak
void TapKeyNatural(WORD scanCode, const std::string& keyName) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> holdDist(30, 80); 

    Log("Input -> Menekan tombol: " + keyName);
    SendKeyByScanCode(scanCode, true);
    Sleep(holdDist(gen));
    SendKeyByScanCode(scanCode, false);
}

// Fungsi khusus untuk Double Tap X (Batal Kerja & Turunkan Tangan)
void CancelAnimation(const std::string& contextLog) {
    Log(contextLog);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    // Jeda dipercepat untuk double-tap cepat (50ms - 150ms)
    std::uniform_int_distribution<> doubleTapDelay(50, 150); 

    TapKeyNatural(SC_X, "X (Cancel Animasi / Hands Up)");
    Sleep(doubleTapDelay(gen)); // Jeda sangat singkat
    TapKeyNatural(SC_X, "X (Stop Hands Up)");
}

// Fungsi Sleep yang bisa di-cancel kapan saja
bool InterruptibleSleep(int targetMilliseconds) {
    int elapsed = 0;
    const int step = 100; 
    
    while (elapsed < targetMilliseconds) {
        if (!isRunning || !isToggled) {
            return false; 
        }
        Sleep(step);
        elapsed += step;
    }
    return true;
}

void MacroThread() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> delay1Sec(1200, 1500);   
    std::uniform_int_distribution<> delay6Sec(6000, 6300);   

    while (isRunning) {
        if (isToggled) {
            Log("=== Memulai Siklus Baru ===");
            
            // 1. Mulai Kerja
            TapKeyNatural(SC_E, "E (Mulai Kerja)");

            // 2. Tunggu sesuai mode
            int waitTime = isTestMode ? 5000 : 1800000;
            std::string modeText = isTestMode ? "5 detik [TEST MODE]" : "30 menit [NORMAL MODE]";
            
            Log("Status -> Sedang bekerja. Menunggu " + modeText + "...");
            if (!InterruptibleSleep(waitTime)) continue; 

            // 3. Batal Animasi (Double Tap X Cepat)
            CancelAnimation("Status -> Waktu tunggu selesai. Double-Tap X (Hands Up & Turun)...");

            // 4. Jeda 1 detik sebelum makan
            if (!InterruptibleSleep(delay1Sec(gen))) continue;

            // 5. Makan (Angka 4)
            TapKeyNatural(SC_4, "4 (Makan)");
            Log("Status -> Proses makan berjalan, menunggu 6 detik...");
            if (!InterruptibleSleep(delay6Sec(gen))) continue;

            // 6. Minum (Angka 5)
            TapKeyNatural(SC_5, "5 (Minum)");
            Log("Status -> Proses minum berjalan, menunggu 6 detik...");
            if (!InterruptibleSleep(delay6Sec(gen))) continue;

            // 7. Batal Animasi (Double Tap X Cepat)
            CancelAnimation("Status -> Waktu tunggu selesai. Double-Tap X (Hands Up & Turun)...");
            Log("=== Siklus Makan & Minum Selesai ===");
            
        } else {
            Sleep(100);
        }
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Siklus Kerja & Survival (GTA V/FiveM)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Fitur: Quick Double-Tap X (Hands Up Batal)" << std::endl;
    std::cout << "[0] - Toggle Makro ON/OFF" << std::endl;
    std::cout << "[7] - Toggle Test Mode (Jeda 5 Detik)" << std::endl;
    std::cout << "[9] - Exit Program" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    std::thread macro(MacroThread);
    bool zeroWasPressed = false;
    bool sevenWasPressed = false;

    while (isRunning) {
        // Cek tombol 9 (Exit)
        if (GetAsyncKeyState(0x39) & 0x8000) {
            Log("Menutup program...");
            isRunning = false;
            CancelAnimation("Force Cancel Animasi (Quick Double Tap X) - EXIT");
            break;
        }

        // Cek tombol 7 (Toggle Test Mode)
        if (GetAsyncKeyState(0x37) & 0x8000) {
            if (!sevenWasPressed) {
                isTestMode = !isTestMode;
                if (isTestMode) {
                    Log("TEST MODE AKTIF -> Durasi tunggu diubah menjadi 5 Detik.");
                } else {
                    Log("TEST MODE NONAKTIF -> Durasi tunggu kembali ke 30 Menit.");
                }
                sevenWasPressed = true;
            }
        } else {
            sevenWasPressed = false;
        }

        // Cek tombol 0 (Toggle Macro)
        if (GetAsyncKeyState(0x30) & 0x8000) {
            if (!zeroWasPressed) {
                isToggled = !isToggled;
                if (isToggled) {
                    Log("TOGGLE ON -> Mengaktifkan makro.");
                } else {
                    Log("TOGGLE OFF -> Makro dihentikan.");
                    CancelAnimation("Force Cancel Animasi (Quick Double Tap X) - OFF");
                }
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
