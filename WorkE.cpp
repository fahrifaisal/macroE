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

// Menekan tombol dengan durasi tahan (hold) acak layaknya manusia
void TapKeyNatural(WORD scanCode, const std::string& keyName) {
    // Randomizer lokal untuk thread-safety
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> holdDist(30, 80); // Tahan tombol 30-80ms

    Log("Input -> Menekan tombol: " + keyName);
    SendKeyByScanCode(scanCode, true);
    Sleep(holdDist(gen));
    SendKeyByScanCode(scanCode, false);
}

// Fungsi Sleep yang bisa di-cancel kapan saja menggunakan tombol toggle/exit
bool InterruptibleSleep(int targetMilliseconds) {
    int elapsed = 0;
    const int step = 100; // Cek status setiap 100ms
    
    while (elapsed < targetMilliseconds) {
        if (!isRunning || !isToggled) {
            return false; // Interupsi terjadi, batalkan sleep
        }
        Sleep(step);
        elapsed += step;
    }
    return true;
}

void MacroThread() {
    std::random_device rd;
    std::mt19937 gen(rd());

    // Distribusi pengacak delay untuk membuat gerakan tidak statis
    std::uniform_int_distribution<> delay1Sec(1000, 1200);   // Jeda 1 detik (+ random)
    std::uniform_int_distribution<> delay6Sec(6000, 6300);   // Jeda 6 detik (+ random)

    while (isRunning) {
        if (isToggled) {
            Log("=== Memulai Siklus Baru ===");
            
            // 1. Mulai Kerja
            TapKeyNatural(SC_E, "E (Mulai Kerja)");

            // 2. Tunggu 30 Menit (1.800.000 ms)
            Log("Status -> Sedang bekerja. Menunggu 30 menit...");
            if (!InterruptibleSleep(1800000)) continue; 

            // 3. Batal Animasi
            Log("Status -> 30 Menit berlalu. Membatalkan animasi kerja...");
            TapKeyNatural(SC_X, "X (Cancel Animasi Kerja)");

            // 4. Jeda 1 detik
            if (!InterruptibleSleep(delay1Sec(gen))) continue;

            // 5. Makan (Angka 4)
            TapKeyNatural(SC_4, "4 (Makan)");
            Log("Status -> Proses makan berjalan, menunggu 6 detik...");
            if (!InterruptibleSleep(delay6Sec(gen))) continue;

            // 6. Minum (Angka 5)
            TapKeyNatural(SC_5, "5 (Minum)");
            Log("Status -> Proses minum berjalan, menunggu 6 detik...");
            if (!InterruptibleSleep(delay6Sec(gen))) continue;

            Log("=== Siklus Makan & Minum Selesai ===");
            // Kembali ke atas loop (akan langsung menekan E lagi karena isToggled masih true)
            
        } else {
            // Idle state, menghemat CPU
            Sleep(100);
        }
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Siklus Kerja & Survival (GTA V/FiveM)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "[0] - Toggle ON/OFF (Mulai/Berhenti Siklus)" << std::endl;
    std::cout << "[9] - Exit Program" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    std::thread macro(MacroThread);
    bool zeroWasPressed = false;

    while (isRunning) {
        // Cek tombol 9 (Exit)
        if (GetAsyncKeyState(0x39) & 0x8000) {
            Log("Menutup program...");
            isRunning = false;
            // Force tekan X sekali saat exit
            TapKeyNatural(SC_X, "X (Force Cancel Animasi - Exit)");
            break;
        }

        // Cek tombol 0 (Toggle)
        if (GetAsyncKeyState(0x30) & 0x8000) {
            if (!zeroWasPressed) {
                isToggled = !isToggled;
                if (isToggled) {
                    Log("TOGGLE ON -> Mengaktifkan makro.");
                } else {
                    Log("TOGGLE OFF -> Makro dihentikan.");
                    // Force tekan X sekali saat toggle dimatikan
                    TapKeyNatural(SC_X, "X (Force Cancel Animasi - OFF)");
                }
                zeroWasPressed = true;
            }
        } else {
            zeroWasPressed = false;
        }

        Sleep(10); // Loop utama berjalan ringan
    }

    // Pastikan thread tertutup dengan aman
    if (macro.joinable()) {
        macro.join();
    }

    return 0;
}
