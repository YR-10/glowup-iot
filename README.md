# Smart Light ESP8266

Firmware PlatformIO/Arduino untuk NodeMCU LoLin: AUTO LDR, MANUAL web, LCD I2C dan PWM MOSFET. Acuan hardware terbaru **memakai XL6009**, bukan SVG tanpa XL6009 yang sebelumnya dibuat. File gambar lama tidak diubah.

## Mulai melalui USB

1. Buka folder ini di Antigravity/VS Code dengan PlatformIO, atau pakai PlatformIO Core.
2. Isi `include/secrets.h` dengan WiFi **2,4 GHz**. Jangan bagikan file tersebut. `secrets.example.h` adalah contoh.
3. Default aman untuk uji tanpa rangkaian: `LAMP_OUTPUT_ENABLED`, `SENSOR_ENABLED`, `LCD_ENABLED`, dan `LDR_CALIBRATED` semuanya `false`.
4. Build: `pio run`. Sambungkan NodeMCU dengan kabel USB data. Upload: `pio run -t upload`. Serial: `pio device monitor -b 115200`. Jika diperlukan tambahkan `--upload-port COMx` saat upload.
5. Buka IP yang dicetak Serial. Jika SSID kosong atau WiFi gagal selama 15 detik, hotspot `SmartLight-<chip-id>` dibuat. Password acak dicetak di Serial setiap boot; IP umumnya `192.168.4.1`. Untuk password tetap, isi `AP_PASSWORD` sepanjang 8–63 karakter.
6. Pilih MANUAL, Nyala, dan geser slider. Target berubah tetapi output aktual tetap 0 selama penguncian aktif. Tidak ada nilai sensor palsu.

Tidak ada upload otomatis dari proyek ini. Jangan hubungkan suplai eksternal dan USB bersamaan sebelum jalur daya board dipastikan. Hanya jaringan lokal tepercaya: API tidak memakai autentikasi pengguna/TLS; jangan port-forward ke internet. Header kontrol membatasi request formulir lintas origin, bukan pengganti autentikasi. Hotspot tetap tersedia setelah WiFi rumah tersambung kembali.

PlatformIO lokal juga disiapkan di `.tools/`. Di komputer ini, PowerShell dapat menjalankan `./scripts/pio-local.ps1 build`, `./scripts/pio-local.ps1 upload`, atau `./scripts/pio-local.ps1 monitor`. Skrip memakai runtime Python Codex lokal; untuk komputer lain gunakan instalasi PlatformIO sendiri.

## Tahapan aktivasi hardware

1. Verifikasi batas A0 board: ADC chip 0–1 V, pembagi board berbeda-beda. LDR VCC 3V3, GND common, AO ke A0 setelah penyesuaian bila diperlukan, DO NC. Baru aktifkan `SENSOR_ENABLED`.
2. Ukur ADC terang dan gelap melalui Serial. Isi `LDR_MIN` dengan angka lebih kecil, `LDR_MAX` lebih besar. Jika ADC turun saat semakin terang, `LDR_INVERTED=true`. Pastikan light dekat 100% saat terang dan 0% saat gelap. Baru set `LDR_CALIBRATED=true`.
3. LCD: D2/GPIO4 SDA, D1/GPIO5 SCL. Gunakan level shifter bidirectional untuk backpack 5 V dengan pull-up 5 V. Setelah wiring benar, aktifkan `LCD_ENABLED`. Alamat default 0x27; sesuaikan ke alamat yang telah diperiksa (misalnya 0x3F). Tidak terdeteksi: LCD dilewati dan Serial melaporkan masalah.
4. Atur LM2596 5,0 V dan XL6009 12,0 V menggunakan multimeter sebelum menyambungkan beban. Tentukan arus LED dan kemampuan termal converter/MOSFET terlebih dahulu.
5. D5/GPIO14 → resistor gate 100–220 ohm → Gate; Gate → 10k → Source/GND; Drain → LED negatif; LED positif → XL6009 OUT+. Semua ground common. Pin fisik wajib mengikuti datasheet MOSFET aktual. IRFZ44N tidak cocok langsung 3,3 V; label logic-level pun belum menjamin Rds(on) pada 3,3 V.
6. Setelah pemeriksaan hardware, ubah `LAMP_OUTPUT_ENABLED=true`, build/upload ulang. Uji MANUAL dengan beban kecil yang sesuai, kemudian AUTO. Pengunci ini tidak bisa dibuka melalui web.

## Daya dan kebutuhan rakit

Adaptor 12V → LM2596 5V → VIN NodeMCU, LCD VCC, XL6009 IN+. XL6009 OUT+ 12V → LED +; OUT− common ground. LED − hanya melalui drain/source MOSFET.

Siapkan NodeMCU dan USB data, LM2596, XL6009, LDR 4 pin, LCD 16x2/backpack, level shifter I2C, MOSFET yang sesuai, resistor gate/pull-down, terminal, perfboard, kabel daya, sekring sesuai perhitungan, multimeter, solder dan isolasi. Jumlah/watt LED, MOSFET aktual, rating sekring/kabel dan kapasitas converter **belum ditentukan**. Firmware tidak mengukur arus, suhu, tegangan rail atau mendeteksi overload. Trimpot XL6009 biasa mengatur tegangan, bukan batas arus. PWM bukan proteksi overload.

## Perilaku kontrol

- Boot segera menarik D5 LOW; resistor fisik 10k diperlukan saat reset sebelum kode berjalan.
- Default AUTO, tetapi tanpa kalibrasi/sensor output AUTO tetap OFF. MANUAL mulai OFF saat pindah mode.
- Sensor setiap 150 ms, EMA alpha 0,15. Persentase dibatasi 0–100.
- AUTO ON bila cahaya ≤70%, OFF bila ≥85%; di antaranya status sebelumnya dipertahankan. Saat ON target = 100 − light. Threshold merupakan nilai awal untuk disesuaikan, bukan kalibrasi lingkungan.
- Output mendekati target 2 poin setiap 100 ms. OFF manual/slider 0 langsung mematikan output. Maksimum transisi 0→100 sekitar 5 detik.
- PWM disetel eksplisit 1023, 1 kHz. LCD setiap 500 ms; status web setiap 750 ms; Serial setiap 1 detik. Interval menggunakan millis tanpa delay panjang.
- LDR tetap dibaca di MANUAL. WiFi gagal tidak menghentikan kontrol. Sensor terlepas tidak selalu dapat dideteksi oleh ADC; kalibrasi bukan deteksi kerusakan sensor.
- Mode/brightness tidak disimpan ke flash: restart kembali ke konfigurasi boot.

## API

GET `/api/status`: mode, adc, light, sensorReady, calibrated, lamp, brightness aktual, target, manualBrightness, manualOn, pwm, outputEnabled, wifi, ap, ip, uptime detik, lcd.

POST memerlukan `X-SmartLight: 1` dan form `application/x-www-form-urlencoded`:

| Endpoint | Field `value` |
| --- | --- |
| `/api/mode` | AUTO atau MANUAL |
| `/api/lamp` | ON atau OFF; MANUAL saja |
| `/api/brightness` | bilangan bulat 0–100; MANUAL saja |

Invalid input → 400; kontrol manual dalam AUTO → 409; header hilang → 403. Status brightness/PWM adalah **perintah output**, bukan konfirmasi cahaya/arus fisik.

## Prosedur uji di perangkat

- USB tanpa beban: boot tidak menghasilkan PWM; dashboard menunjukkan pengunci. Request brightness tetap tidak membuka pengunci.
- WiFi kosong/salah: hotspot muncul, password Serial bekerja, API dapat diakses.
- API: tolak -1, 101, teks, kosong; tolak kontrol lampu dalam AUTO. Pindah MANUAL, ON, slider, OFF; target dan output harus dibedakan.
- Kalibrasi: terang→light tinggi, gelap→light rendah; AUTO OFF tanpa kalibrasi. Setelah kalibrasi, periksa kedua threshold dan kestabilan di daerah antar-threshold.
- MANUAL: perubahan cahaya tidak mengubah target manual. OFF harus segera 0.
- Cabut koneksi WiFi: AUTO tetap berjalan; fallback tersedia. Restart: kembali ke default.
- Setelah output diaktifkan: ukur gate, rail dan arus LED dengan alat yang sesuai; mulai dari beban kecil, periksa suhu. Tidak mengklaim hardware aman dari hasil build.

## Struktur

## Hasil pemeriksaan 6 Oktober 2026

- Build PlatformIO nodemcuv2 berhasil, Arduino ESP8266 3.1.2; RAM 29.968/81.920 byte (36,6%), flash 316.007/1.044.464 byte (30,3%). BIN: `.pio/build/nodemcuv2/firmware.bin`.
- Sintaks JavaScript dashboard diperiksa dengan Node.js; lolos. `preview/dashboard.html` adalah pratinjau statis dan tidak menyediakan API perangkat.
- Ada peringatan dari library LCD/tool bawaan; build selesai sukses.
- Belum upload, belum uji runtime ESP8266/LCD/WiFi/PWM, belum uji listrik. Build berhasil tidak menyatakan hardware terverifikasi.

## Berkas

`include/config.h`: parameter; `pins.h`: mapping; `light_control.h`: filter/mapping/kontrol; `secrets.h`: kredensial lokal; `src/main.cpp`: integrasi; `src/web_ui.h`: UI dalam flash.

Referensi: https://arduino-esp8266.readthedocs.io/en/latest/reference.html dan https://docs.platformio.org/en/latest/boards/espressif8266/nodemcuv2.html
