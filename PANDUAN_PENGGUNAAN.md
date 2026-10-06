# 📖 Panduan Lengkap Penggunaan Proyek Smart Light ESP8266

Panduan ini dibuat khusus agar kamu (atau rekan tim) dapat merakit, memprogram, menguji, dan menggunakan sistem **Smart Light ESP8266** dengan mudah, aman, dan tanpa kendala.

---

## 🌟 Tentang Proyek Ini

Sistem lampu pintar berbasis **NodeMCU ESP8266** dengan fitur:
* **Mode Otomatis (AUTO)**: Menyesuaikan intensitas cahaya LED strip secara halus (*smooth PWM ramping*) berdasarkan tingkat kegelapan yang dibaca oleh sensor LDR.
* **Mode Manual (MANUAL)**: Mengatur ON/OFF dan intensitas lampu (0–100%) melalui Web Dashboard dari HP atau laptop.
* **LCD 16x2 I2C**: Menampilkan telemetri real-time (kondisi sensor, status lampu, dan mode) dengan deteksi alamat I2C otomatis.
* **WiFi & Hotspot Fallback**: Terhubung ke WiFi rumah, atau otomatis membuka Hotspot sendiri (`SmartLight-<id>`) jika tidak ada internet/WiFi.

---

## 🛠️ Daftar Komponen yang Dibutuhkan

| No | Komponen | Fungsi |
| :---: | :--- | :--- |
| 1 | **NodeMCU LoLin ESP8266 (V3)** | Mikrokontroler utama dengan WiFi terintegrasi |
| 2 | **Modul Sensor Cahaya (LDR) 4 Pin** | Mendeteksi intensitas cahaya ruangan |
| 3 | **Modul LCD 16x2 + I2C Backpack (PCF8574)** | Menampilkan status sistem |
| 4 | **Modul Step-Down LM2596** | Menurunkan tegangan Adaptor 12V ke 5V (untuk NodeMCU, LCD, dan XL6009) |
| 5 | **Modul Step-Up XL6009** | Mengubah 5V menjadi 12V (untuk daya LED strip) |
| 6 | **MOSFET IRLZ44N / IRLZ34N (Logic-Level)** | Sakelar elektronik PWM untuk LED Strip 12V |
| 7 | **Resistor 10kΩ & 100Ω–220Ω** | Resistor pull-down Gate ke GND (10k) & resistor Gate (100–220Ω) |
| 8 | **LED Strip 12V DC** | Beban lampu utama |
| 9 | **Adaptor 12V 2A DC** | Sumber daya utama seluruh rangkaian |

---

## 🔌 Skema Wiring (Pin Mapping)

> ⚠️ **PERINGATAN KESELAMATAN SEBELUM COLOK KABEL:**
> 1. **Ukur tegangan modul sebelum dipasang:**
>    - Putar trimpot **LM2596** hingga output tepat **5.0V** menggunakan multimeter sebelum menyambungkannya ke NodeMCU.
>    - Putar trimpot **XL6009** hingga output tepat **12.0V** sebelum menyambungkannya ke LED Strip.
> 2. **VCC Sensor LDR WAJIB ke 3V3 NodeMCU:** Jangan sambungkan VCC LDR ke 5V karena pin A0 NodeMCU maksimal menerima 3,3V.
> 3. **Common Ground:** Semua kabel minus/GND (Adaptor, LM2596, XL6009, NodeMCU, dan Source MOSFET) harus saling terhubung.

### Tabel Koneksi Pin:

| Dari Komponen | Pin Modul | Ke NodeMCU / Komponen Lain | Keterangan / Warna Kabel |
| :--- | :---: | :--- | :--- |
| **Adaptor 12V** | `+12V` | LM2596 `IN+` | Kabel Merah |
| **Adaptor 12V** | `GND` | LM2596 `IN-` | Kabel Hitam (GND) |
| **LM2596 (Output 5V)** | `OUT+` | NodeMCU `VIN`, LCD `VCC`, XL6009 `IN+` | Kabel Kuning (+5V) |
| **LM2596 (Output 5V)** | `OUT-` | NodeMCU `G`, XL6009 `IN-`, LCD `GND` | Kabel Hitam (GND) |
| **XL6009 (Output 12V)**| `OUT+` | LED Strip `Kutub Positif (+)` | Kabel Merah (+12V) |
| **XL6009 (Output 12V)**| `OUT-` | Common GND | Kabel Hitam (GND) |
| **LED Strip 12V** | `Kutub Negatif (-)` | MOSFET Pin **D (Drain)** | Kabel Hitam |
| **MOSFET (IRLZ44N)** | Pin **G (Gate)** | NodeMCU **D5 (GPIO14)** via R 100-220Ω | Kabel Hijau (PWM) *(ada R 10k ke GND)* |
| **MOSFET (IRLZ44N)** | Pin **S (Source)**| Common GND | Kabel Hitam (GND) |
| **Sensor LDR (4-Pin)** | `VCC` | NodeMCU **3V3** | Kabel Oranye (3.3V) |
| **Sensor LDR (4-Pin)** | `GND` | NodeMCU **G (GND)** | Kabel Hitam (GND) |
| **Sensor LDR (4-Pin)** | `AO` | NodeMCU **A0** | Kabel Cyan (Analog Output) |
| **Sensor LDR (4-Pin)** | `DO` | *(Tidak Digunakan / Biarkan Kosong)* | - |
| **LCD 16x2 I2C** | `SDA` | NodeMCU **D2 (GPIO4)** | Kabel Biru (I2C SDA) |
| **LCD 16x2 I2C** | `SCL` | NodeMCU **D1 (GPIO5)** | Kabel Ungu (I2C SCL) |

---

## 💻 Langkah Membuka & Memprogram di Laptop

### 1. Kebutuhan Software
* Pasang **[Visual Studio Code (VS Code)](https://code.visualstudio.com/)**.
* Di VS Code, pasang ekstensi: **PlatformIO IDE**.
* Pasang driver USB **CH340** (jika laptop belum mengenali board NodeMCU saat dicolok).

### 2. Membuka Proyek
1. Buka VS Code.
2. Pilih menu **File > Open Folder...**, lalu pilih folder proyek ini (`arduino-esp8266`).

### 3. Mengatur WiFi (Opsional)
1. Buka file `include/secrets.example.h`.
2. Buat file baru bernama `include/secrets.h` (atau edit file tersebut jika sudah ada), lalu isi dengan WiFi rumahmu:
   ```cpp
   #pragma once
   constexpr char WIFI_SSID[] = "NamaWiFiKamu";
   constexpr char WIFI_PASSWORD[] = "PasswordWiFiKamu";
   constexpr char AP_PASSWORD[] = "12345678"; // Password hotspot jika WiFi rumah mati
   ```
   *Catatan: Jika dikosongkan `""`, ESP8266 akan otomatis membuat Hotspot sendiri tanpa password.*

### 4. Upload Firmware ke NodeMCU
1. Colokkan board NodeMCU ke laptop menggunakan kabel data micro-USB *(adaptor 12V jangan dicolokkan dulu)*.
2. Di pojok kiri bawah VS Code (bilah status PlatformIO), klik tombol panah kanan **`PlatformIO: Upload`** (atau tekan tombol centang untuk build terlebih dahulu).
3. Tunggu hingga proses flashing selesai dan muncul tulisan **`[SUCCESS]`**.

---

## 📱 Cara Menggunakan Web Dashboard

1. **Jika Terhubung ke WiFi Rumah:**
   * Buka Serial Monitor (baud rate **115200**) di VS Code untuk melihat alamat IP yang didapat (misal: `192.168.1.50`).
   * Buka browser di HP/Laptop yang terhubung ke WiFi yang sama, lalu ketikkan IP tersebut di address bar.
2. **Jika Menggunakan Hotspot Bawaan ESP8266:**
   * Sambungkan WiFi HP ke hotspot bernama: **`SmartLight-XXXXXX`**.
   * Buka browser di HP, lalu buka alamat: **`http://192.168.4.1`**.
3. **Fitur di Dashboard:**
   * **Tombol AUTO**: Lampu akan menyala dan redup secara otomatis menyesuaikan gelap-terang ruangan.
   * **Tombol MANUAL**: Kamu bisa menggeser slider (0–100%) atau menekan tombol **Nyala** / **Matikan**.
   * **Kotak Info**: Melihat nilai sensor LDR, ADC mentah, daya PWM, dan waktu aktif perangkat.

---

## ⚙️ Langkah Kalibrasi & Aktivasi Bertahap

Untuk keamanan maksimal, firmware ini memiliki fitur **Safety Lock** di file [`include/config.h`](file:///c:/Users/User/arduino-esp8266/include/config.h):

1. **Uji Coba Awal (Hanya USB Laptop):**
   * Default kode: `LAMP_OUTPUT_ENABLED = false`.
   * Pada mode ini, kamu bisa membuka web dashboard dan menggeser slider dengan aman tanpa khawatir board menarik arus berlebih dari laptop.
2. **Kalibrasi Sensor LDR:**
   * Buka dashboard web, perhatikan angka **ADC mentah**.
   * Tutup sensor LDR rapat-rapat dengan jari, catat angka ADC-nya (misal: `120`).
   * Sinari LDR dengan senter HP, catat angka ADC-nya (misal: `850`).
   * Buka `include/config.h`, masukkan angka tersebut:
     ```cpp
     constexpr int LDR_MIN = 120;
     constexpr int LDR_MAX = 850;
     constexpr bool LDR_CALIBRATED = true;
     ```
   * *Catatan: Jika saat disenter angka ADC justru makin kecil, ubah `LDR_INVERTED = true`.*
3. **Aktivasi LCD 16x2:**
   * Sambungkan kabel LCD ke D1 dan D2.
   * Di `include/config.h`, ubah:
     ```cpp
     constexpr bool LCD_ENABLED = true;
     ```
   * Firmware sudah dilengkapi **Auto-Scan I2C**, jadi modul beralamat `0x27` maupun `0x3F` akan langsung terdeteksi otomatis.
4. **Aktivasi Output Lampu Fisik:**
   * Pastikan rangkaian daya adaptor 12V, LM2596 (5V), XL6009 (12V), MOSFET, dan LED Strip sudah terpasang rapi dan dicek dengan multimeter.
   * Di `include/config.h`, ubah:
     ```cpp
     constexpr bool LAMP_OUTPUT_ENABLED = true;
     ```
   * Upload ulang firmware. Sekarang lampu LED strip 12V sudah aktif penuh dan siap digunakan!

---

*Semoga panduan ini membantu! Jika ada pertanyaan mengenai hardware atau kode, periksa file `include/config.h` dan `include/pins.h`.*
