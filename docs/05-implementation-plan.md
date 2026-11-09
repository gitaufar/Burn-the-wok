# 05 — Implementation Plan

Pengembangan dibagi **7 tahap**. Karena engine dibuat sendiri (syarat nilai 99), Tahap 1–2 memuat pekerjaan engine, bukan hanya gameplay. **Setiap akhir tahap harus menghasilkan build yang bisa dijalankan.**

## Jadwal (dari RPS)

| Sesi RPS | Tanggal |
|---|---|
| Implementasi 1 | 06/11 |
| Implementasi 2 | 09/11 |
| Implementasi 3 | 13/11 |
| Implementasi 4 | 16/11 |
| Implementasi 5 | 20/11 |
| Implementasi 6 | 23/11 |
| Finalisasi Dokumen Laporan | 27/11 (paralel) |
| Play Testing | 30/11 |
| UAS | menyusul |

## Tahap 1 — Engine core (06/11)

Fokus: fondasi engine, belum gameplay.
- [x] Setup solution: proyek **Engine** terpisah dari **Game** (`Engine/` static lib, `Burn The Wok/` exe, SDL2 + GLEW di `External/`)
- [x] Window + konteks grafis, game loop (fixed timestep update, render terpisah)
- [x] Input system (keyboard & mouse; down/pressed/released)
- [x] Renderer 2D: tekstur, sprite, sprite batch, atlas, filter nearest. _Catatan: tekstur dibuat dari piksel di kode; memuat file PNG menunggu keputusan decoder._
- [x] Kamera 2D, konversi layar↔dunia (highlight tile di bawah mouse)
- [x] Scene/State manager (Menu, Gameplay, Menang, Kalah — Menang/Kalah = `ResultScene`)
- [x] Demo: sprite pemain (placeholder) bergerak 4 arah di peta tile

**DoD:** pemain berjalan di peta hutan placeholder; engine berdiri terpisah dari kode Game. **TERCAPAI 06/11/2026.**

Verifikasi 06/11/2026: build Debug & Release 0 warning; dijalankan di GPU NVIDIA (OpenGL 3.3 core); diuji: Menu → Gameplay (Enter) → Menang (F1) → Menu (Enter) → keluar (Esc), pemain bergerak ke kanan & kamera mengikuti, build Release jalan tanpa konsol. **Belum diuji otomatis:** zoom roda mouse, F2 (Kalah), resize jendela, GPU Intel/AMD — mohon dicoba manual oleh tim.

Perbaikan startup 06/11/2026: menu awal kini menampilkan judul, tombol **MULAI** yang bisa diklik, dan petunjuk keyboard di layar. Input menyimpan event tekan/lepas sampai fixed update agar ketukan singkat tidak terlewat; Enter numpad juga didukung di menu dan layar hasil. Build Debug dan Release berhasil. Pemeriksaan runtime lokal dengan pasangan event tekan/lepas dalam satu polling: Enter, Spasi, Enter numpad, dan klik MULAI semuanya berhasil berpindah ke Gameplay; tidak ditemukan error OpenGL pada pemeriksaan tersebut. Harness pemeriksaan disimpan di folder output `Debug/` yang diabaikan Git.

Pembaruan visual 06/11/2026: aset pohon dipakai pada setiap tile pohon dengan ukuran maksimal 16x16 piksel dunia; jumlah dan lokasi tile pohon mengikuti generator peta yang sama. Tampilan gameplay mendapat variasi rumput, bunga, jalur tanah dekoratif, batu, semak, tanaman tepian kolam, riak air animasi, bayangan pohon/pemain, partikel daun, dan vignette tipis. Overlay menampilkan minimap aktual, area kamera, posisi pemain, koordinat, zoom, serta petunjuk kontrol. Sprite penjaga hutan mendapat palet lebih jelas dan gerakan naik-turun saat berjalan. Detail tambahan dibuat lewat kode dengan aset PNG yang sudah ada, tanpa dependency baru.

## Tahap 2 — Collision & mekanik air/api (09/11)

- [ ] Entity/object system sederhana
- [ ] Tilemap (pohon, tanah, sumber air)
- [ ] Collision detection (AABB/lingkaran) + query area
- [ ] Objek Api (intensitas/HP)
- [ ] Penyemprot air: arah dari mouse, partikel/semprotan, collision dengan api
- [ ] Sistem air: kapasitas, berkurang saat menyemprot, isi ulang di sumber air

**DoD:** pemain bisa memadamkan api dan mengisi ulang air.

## Tahap 3 — Penyebaran api & perubahan lahan (13/11)

- [ ] Api menyebar ke pohon sekitar
- [ ] State pohon: sehat → terbakar → (jika dibiarkan) sawit; juga hangus jika perlu
- [ ] Timer stage
- [ ] Persentase hutan tersisa
- [ ] Kondisi menang/kalah dasar (batas sawit, api padam)

**DoD:** loop inti jalan: api menyebar, pohon berubah jadi sawit, gim bisa menang atau kalah.

## Tahap 4 — Satwa & AI (16/11)

- [ ] Orang utan, bekantan, burung hantu (entitas + sprite 4 arah)
- [ ] AI state machine: diam → panik → kabur → selamat / terbakar
- [ ] Mekanik penyelamatan satwa, target satwa per stage
- [ ] Integrasi target satwa ke kondisi menang/kalah

**DoD:** target satwa terpenuhi/gagal dan memengaruhi hasil.

## Tahap 5 — Sabotase & 3 stage (20/11)

- [ ] AI anak buah: menyebar api, menanam bibit sawit (stage 2)
- [ ] AI Wowo & Tedduy: menghadang pemain (stage 3)
- [ ] Konfigurasi 3 stage (data-driven, satu file config)
- [ ] Kurva kesulitan: api lebih cepat, hambatan lebih banyak, waktu lebih singkat

**DoD:** tiga stage playable berurutan dengan kesulitan naik.

## Tahap 6 — UI, audio, polish (23/11)

- [ ] UI system: menu utama, HUD (air, waktu, % hutan, satwa), pause
- [ ] Layar menang/kalah + penilaian 1–3 bintang
- [ ] Audio: SFX & BGM
- [ ] Partikel api & air
- [ ] Ganti semua placeholder dengan aset pixel art final

**DoD:** gim terlihat dan terdengar utuh, siap diuji.

## Tahap 7 — Play testing & finalisasi (30/11)

- [ ] Uji ke teman sekelas, catat masukan
- [ ] Balancing (kecepatan api, kapasitas air, jumlah satwa, durasi)
- [ ] Perbaikan bug, optimasi
- [ ] Build rilis Windows (HTML5 hanya jika sangat longgar & tetap tanpa engine pihak ke-3)
- [ ] Hasil play testing masuk laporan

**DoD:** build final stabil untuk presentasi/UAS.

## Paralel: laporan (finalisasi 27/11)

- Catat progres, screenshot, dan catatan teknis di akhir tiap tahap.
- Susun bab arsitektur engine (bukti "engine buatan sendiri").
- Cantumkan Disclaimer Penggunaan AI yang jujur.

## Risiko

| Risiko | Mitigasi |
|---|---|
| Tahap 1–2 molor karena engine dari nol | Batasi fitur engine ke yang benar-benar dipakai gim; jangan buat engine "lengkap" |
| Penyebaran api (T3) & AI sabotase (T5) rumit | Mulai dari aturan sederhana (grid, timer); stage 3 boleh disederhanakan jadi hambatan + api lebih cepat |
| Audio dari nol berat | Gunakan library level rendah jika disetujui dosen |
| Aset final telat | Pasang aset bertahap sejak T3 |
| Rubrik penilaian lengkap & status decoder gambar/audio belum pasti | Tanyakan ke dosen (lihat `02-grading-constraints.md`) |

## Status saat ini

- Tahap aktif: **1 selesai** (06/11/2026); berikutnya **Tahap 2** (09/11), menunggu persetujuan tim
- Terakhir diperbarui: 06/11/2026
- Kondisi repo (06/11/2026): solution `Engine` (static lib) + `Burn The Wok` (exe), C++20/MSVC v145/Win32, SDL2 + GLEW di `External/`. Build Debug & Release sukses; demo Tahap 1 bisa dimainkan. Aset masih placeholder yang digambar lewat kode. Git sudah di-init, **belum ada commit**.
- Status per tahap:

| Tahap | Status |
|---|---|
| 1 — Engine core | **Selesai** (06/11/2026) |
| 2 — Collision & air/api | Belum mulai |
| 3 — Penyebaran api & lahan | Belum mulai |
| 4 — Satwa & AI | Belum mulai |
| 5 — Sabotase & 3 stage | Belum mulai |
| 6 — UI, audio, polish | Belum mulai |
| 7 — Play testing & finalisasi | Belum mulai |

## Pertanyaan terbuka (06/11/2026)

1. Decoder PNG: SOIL (seperti contoh dosen) atau stb_image? **Perlu konfirmasi singkat ke dosen** (kategori b). Sampai ada keputusan, tekstur dibuat lewat kode.
2. Remote git: _menunggu tim_.
3. Ukuran tile/sprite final (sekarang placeholder 16×16): tentukan bersama artis saat aset mulai dibuat.

Sudah terjawab 06/11/2026 (lihat Log Keputusan): windowing (SDL2 + GLEW), lokasi dependency (`External/`), git init, aset (belum ada), konvensi kode, Console/Windows, Win32 saja, rencana audio.

## Log Keputusan

| Tanggal | Keputusan | Alasan |
|---|---|---|
| 06/11/2026 | Target nilai 99 → engine buatan sendiri | Plafon nilai (lihat 02) |
| 06/11/2026 | Binding OpenGL & windowing dikonfirmasi tetap dihitung engine sendiri | Konfirmasi dosen |
| 06/11/2026 | Karakter pemain = penjaga hutan (pemain sendiri), terpisah dari Wowo/Tedduy | Keputusan tim |
| 06/11/2026 | Bahasa/stack: **C++20**, compiler **MSVC v145** (Visual Studio Community 2026), build **MSBuild** (`.vcxproj` + `.slnx`), target Windows Win32 | Sesuai proyek yang sudah dibuat tim di repo |
| 06/11/2026 | Windowing + OpenGL: **SDL2 (core) + GLEW**, file disalin dari contoh dosen. SDL hanya untuk window/konteks/input; renderer 2D buatan sendiri (tanpa `SDL_Renderer`) | Sama dengan contoh dosen; kategori dikonfirmasi dosen |
| 06/11/2026 | Hanya platform **Win32** (konfigurasi x64 dihapus) | Binary SDL2/GLEW dari contoh dosen hanya x86 |
| 06/11/2026 | SubSystem: **Console** untuk Debug, **Windows** untuk Release | Debug butuh log; Release rapi untuk presentasi |
| 06/11/2026 | Audio: SDL2 core + mixer & loader WAV buatan sendiri; **tanpa** FMOD/SDL_mixer. Decoder PNG: SOIL/stb_image setelah tanya dosen | Mixer audio wajib buatan sendiri (lihat 02) |
| 06/11/2026 | Tahap 1 selesai; tile placeholder 16×16 px, zoom 3× | Demo berjalan; ukuran final menunggu aset |
| 06/11/2026 | Dependency pihak ke-3 di-vendor di repo, folder `External/` | Paling mudah untuk kolaborasi (clone → build tanpa instal); sama dengan pola contoh dosen |
| 06/11/2026 | Repo git dibuat (branch `main`) + `.gitignore`; remote menyusul | Bukti commit bertahap (lihat 02) |
| 06/11/2026 | Konvensi kode di `06-architecture.md` disetujui | Keputusan tim |
| 06/11/2026 | Contoh dosen = kode buku *Game Programming in C++* (Madhav); dipakai sebagai referensi, **kodenya tidak disalin** | Aturan engine sendiri |
