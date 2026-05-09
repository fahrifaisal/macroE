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
std::atomic<int> currentJob(0); // 0 = Menu, 1 = Recycle, 2 = Menambang

// Konstanta Scan Codes untuk DirectInput Games
const WORD SC_1 = 0x02; // Angka 1 (Equip/Unequip)
const WORD SC_4 = 0x05; // Angka 4 (Makan)
const WORD SC_5 = 0x06; // Angka 5 (Minum)
const WORD SC_E = 0x12; // Huruf E
const WORD SC_X = 0x2D; // Huruf X

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
    std::uniform_int_distribution<> doubleTapDelay(50, 150); 

    TapKeyNatural(SC_X, "X (Cancel Animasi / Hands Up)");
    Sleep(doubleTapDelay(gen)); 
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
        if (isToggled && currentJob != 0) {
            Log("=== Memulai Siklus " + std::string(currentJob == 1 ? "RECYCLE" : "MENAMBANG") + " ===");
            
            // ==========================================
            // LANGKAH 1: PERSIAPAN & MULAI KERJA
            // ==========================================
            if (currentJob == 2) {
                // Khusus Menambang: Equip palu dulu
                TapKeyNatural(SC_1, "1 (Equip Palu Tambang)");
                Log("Status -> Menunggu animasi equip palu...");
                if (!InterruptibleSleep(1500)) continue; // Tunggu animasi palu keluar
            }

            TapKeyNatural(SC_E, "E (Mulai Kerja)");

            // ==========================================
            // LANGKAH 2: TUNGGU KERJA (30 Mnt / 5 Dtk)
            // ==========================================
            int waitTime = isTestMode ? 5000 : 1800000;
            std::string modeText = isTestMode ? "5 detik [TEST MODE]" : "30 menit [NORMAL MODE]";
            
            Log("Status -> Sedang bekerja. Menunggu " + modeText + "...");
            if (!InterruptibleSleep(waitTime)) continue; 

            // ==========================================
            // LANGKAH 3: BATAL KERJA & UNEQUIP
            // ==========================================
            CancelAnimation("Status -> Waktu tunggu selesai. Double-Tap X (Hands Up & Turun)...");
            if (!InterruptibleSleep(delay1Sec(gen))) continue;

            if (currentJob == 2) {
                // Khusus Menambang: Simpan palu sebelum makan
                TapKeyNatural(SC_1, "1 (Unequip Palu Tambang)");
                Log("Status -> Menunggu animasi menyimpan palu...");
                if (!InterruptibleSleep(1500)) continue; 
            }

            // ==========================================
            // LANGKAH 4: MAKAN & MINUM
            // ==========================================
            TapKeyNatural(SC_4, "4 (Makan)");
            Log("Status -> Proses makan berjalan, menunggu 6 detik...");
            if (!InterruptibleSleep(delay6Sec(gen))) continue;

            TapKeyNatural(SC_5, "5 (Minum)");
            Log("Status -> Proses minum berjalan, menunggu 6 detik...");
            if (!InterruptibleSleep(delay6Sec(gen))) continue;

            CancelAnimation("Status -> Waktu selesai. Double-Tap X (Clear Animation)...");
            Log("=== Siklus Selesai, Mengulang Kembali ===");
            
        } else {
            Sleep(100);
        }
    }
}

// Fungsi untuk mencetak UI Menu Utama
void PrintMenu() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "        PILIH PEKERJAAN (AIO MACRO)     " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "[1] - Recycle Worker" << std::endl;
    std::cout << "[2] - Menambang (Mining)" << std::endl;
    std::cout << "[9] - Exit Program" << std::endl;
    std::cout << "Pilih angka di keyboard Anda..." << std::endl;
}

// Fungsi untuk mencetak UI Kontrol saat pekerjaan dipilih
void PrintControls() {
    std::cout << "\n----------------------------------------" << std::endl;
    std::cout << "           KONTROL PEKERJAAN            " << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "[0] - Toggle Makro ON/OFF" << std::endl;
    std::cout << "[7] - Toggle Test Mode (Jeda 5 Detik)" << std::endl;
    std::cout << "[6] - Kembali ke Menu Pilihan Pekerjaan" << std::endl;
    std::cout << "[9] - Exit Program" << std::endl;
    std::cout << "Tekan [0] untuk mulai menjalankan makro." << std::endl;
}

int main() {
    PrintMenu();

    std::thread macro(MacroThread);
    
    // Variabel Debounce agar tombol tidak terdeteksi ganda
    bool key0 = false, key1 = false, key2 = false, key6 = false, key7 = false;

    while (isRunning) {
        // Cek tombol 9 (Exit) - Berlaku di mana saja
        if (GetAsyncKeyState(0x39) & 0x8000) {
            Log("Menutup program...");
            isRunning = false;
            CancelAnimation("Force Cancel Animasi - EXIT");
            break;
        }

        if (currentJob == 0) {
            // ==============================
            // MODE MENU: Menunggu Input 1 / 2
            // ==============================
            if (GetAsyncKeyState(0x31) & 0x8000) { // Angka 1
                if (!key1) {
                    currentJob = 1;
                    Log("BERHASIL MEMILIH: [1] Recycle Worker");
                    PrintControls();
                    key1 = true;
                }
            } else { key1 = false; }

            if (GetAsyncKeyState(0x32) & 0x8000) { // Angka 2
                if (!key2) {
                    currentJob = 2;
                    Log("BERHASIL MEMILIH: [2] Menambang");
                    PrintControls();
                    key2 = true;
                }
            } else { key2 = false; }

        } else {
            // ==============================
            // MODE KERJA: Kontrol 0, 6, 7
            // ==============================
            
            // Cek tombol 6 (Kembali ke menu)
            if (GetAsyncKeyState(0x36) & 0x8000) {
                if (!key6) {
                    isToggled = false; // Matikan makro jika sedang jalan
                    currentJob = 0;    // Reset ke menu
                    Log("KEMBALI KE MENU UTAMA. Makro dihentikan.");
                    CancelAnimation("Force Cancel Animasi - KEMBALI KE MENU");
                    PrintMenu();
                    key6 = true;
                }
            } else { key6 = false; }

            // Cek tombol 7 (Toggle Test Mode)
            if (GetAsyncKeyState(0x37) & 0x8000) {
                if (!key7) {
                    isTestMode = !isTestMode;
                    if (isTestMode) Log("TEST MODE AKTIF -> Durasi tunggu diubah menjadi 5 Detik.");
                    else Log("TEST MODE NONAKTIF -> Durasi tunggu kembali ke 30 Menit.");
                    key7 = true;
                }
            } else { key7 = false; }

            // Cek tombol 0 (Toggle Macro)
            if (GetAsyncKeyState(0x30) & 0x8000) {
                if (!key0) {
                    isToggled = !isToggled;
                    if (isToggled) {
                        Log("TOGGLE ON -> Mengaktifkan makro.");
                    } else {
                        Log("TOGGLE OFF -> Makro dihentikan sementara.");
                        CancelAnimation("Force Cancel Animasi - OFF");
                    }
                    key0 = true;
                }
            } else { key0 = false; }
        }

        Sleep(10); // Ringankan pemakaian CPU
    }

    if (macro.joinable()) {
        macro.join();
    }

    return 0;
}
