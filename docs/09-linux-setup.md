# 09 — Setup lokal Linux

Build Linux memakai C++20 + CMake dan library sistem SDL2, GLEW, OpenGL, serta libpng untuk membaca aset PNG. Renderer, game loop, input, collision, dan mixer audio tetap buatan tim. Build Windows tetap melalui solution Visual Studio dan memakai Windows Imaging Component untuk gambar.

## Dependency sistem (Ubuntu 24.04)

```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libglew-dev libgl-dev libpng-dev
```

Tidak perlu menjalankan instalasi lagi jika paket-paket itu sudah tersedia.

## Menjalankan

Dari root repo:

```bash
./run.sh
```

| Perintah | Fungsi |
|---|---|
| `./run.sh` | Build Debug lalu main |
| `./run.sh build` | Build Debug saja |
| `./run.sh release` | Build Release lalu main |
| `./run.sh play` | Main dengan build terakhir |
| `./run.sh run --skip-intro` | Build lalu langsung ke menu |

Hasil build ada di `build/linux/burn-the-wok`. Launcher menyalin aset ke folder build dan mengatur working directory agar gambar, narasi intro, dan musik terbaca. Launcher dapat dipanggil dari direktori mana saja dengan path absolutnya. Gunakan launcher saat main; binary langsung perlu dijalankan dari folder build agar semua path audio dan narasi terbaca.

Build manual:

```bash
cmake -S . -B build/linux -DCMAKE_BUILD_TYPE=Debug
cmake --build build/linux --parallel 4
cd build/linux
./burn-the-wok
```

## Kontrol

- Enter/Spasi: mulai game.
- WASD/panah: bergerak.
- Tahan klik kiri: semprot air ke arah kursor.
- Berdiri di tepi kolam: isi ulang air.
- Roda mouse: zoom.
- Esc: lewati intro / pause atau lanjutkan gameplay / keluar dari menu. Panel pause menyediakan Lanjutkan, Ulangi Stage, dan Kembali ke Menu.

## Jika gagal

- `Could not find SDL2/GLEW/PNG/OpenGL`: periksa paket development di atas.
- `SDL_CreateWindow failed` atau tidak bisa membuka display: jalankan dari sesi desktop lokal yang aktif.
- `OpenGL 3.3 context creation failed`: periksa driver GPU; game membutuhkan OpenGL 3.3 core.
- `LoadSound: cannot open ...`: jalankan melalui `run.sh`, supaya working directory benar.

## Verifikasi lokal (17/11/2026)

Build Debug berhasil pada Ubuntu 24.04 dengan GCC 13.3, CMake 3.28, dan GPU AMD Radeon/OpenGL 4.6. Intro, menu, gameplay, layar hasil (F1), kembali ke menu, dan keluar normal (kode 0) sudah diuji. Seluruh 139 PNG aset berhasil dimuat, termasuk alpha RGBA dan urutan baris pada fixture terpisah; file hilang/rusak ditolak. Log runtime tidak menunjukkan error pemuatan aset/audio. Ini pemeriksaan startup dan navigasi dasar; balancing seluruh campaign belum diuji.

## Aplikasi desktop Iterasi 11

Aplikasi lokal `Burn The Wo` dan ikon Desktop/menu aplikasi sudah diperbarui ke build Release Iterasi 11 pada 20/11/2026. Esc membuka panel pause dengan Lanjutkan, Ulangi Stage, dan Kembali ke Menu. Catatan uji: [verifikasi Iterasi 11](09-iteration-11-verification.md).
