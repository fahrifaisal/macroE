#include <windows.h>
#include <iostream>

int main() {
    bool active = false;
    // VK_F4 adalah tombol F4, 'W' dan 'E' adalah kodenya
    
    std::cout << "Program Ready. Tekan F4 untuk Toggle ON/OFF." << std::endl;

    while (true) {
        // Cek jika F4 ditekan
        if (GetAsyncKeyState(VK_F4) & 1) {
            active = !active;
            if (active) {
                std::cout << "Status: ON (Hold W + Spam E)" << std::endl;
                // Mulai Tahan W
                keybd_event(0x57, 0, 0, 0); 
            } else {
                std::cout << "Status: OFF" << std::endl;
                // Lepas W dan E (safety)
                keybd_event(0x57, 0, KEYEVENTF_KEYUP, 0);
                keybd_event(0x45, 0, KEYEVENTF_KEYUP, 0);
            }
        }

        if (active) {
            // Spam E (Tekan dan Lepas)
            keybd_event(0x45, 0, 0, 0); // E Down
            Sleep(30);                  // Jeda tekan
            keybd_event(0x45, 0, KEYEVENTF_KEYUP, 0); // E Up
            
            // Jeda antar spam agar tidak terdeteksi bot/spam filter
            Sleep(50); 
        } else {
            Sleep(100); // Hemat CPU saat idle
        }
    }
    return 0;
}
