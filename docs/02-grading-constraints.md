# 02 — Grading Constraints (WAJIB DIBACA)

## Rubrik nilai maksimal

| Cara pembuatan gim | Nilai maksimal |
|---|---|
| Game engine **buatan sendiri** | **99** |
| Game engine **pihak ke-3** | 89 |
| Game maker **tanpa coding** | 79 |

**Target tim: 99 → engine buatan sendiri.** Angka di atas adalah *plafon*, bukan jaminan. Nilai akhir tetap bergantung pada kualitas gim, laporan, dan presentasi.

## Aturan untuk Claude Code

### Dilarang
- Menambah/menggunakan game engine atau game framework pihak ke-3: Unity, Godot, Unreal, MonoGame, FNA, Stride, libGDX, Raylib, SFML, Cocos, Phaser/PixiJS/Three.js (jika versi web), dll.
- Menyalin atau mem-fork source engine open source ke repo lalu mengklaimnya sebagai engine sendiri.
- Menambah physics engine / library collision / library UI / library AI siap pakai. Semuanya ditulis sendiri (ini juga materi kuliah).

### Harus ditulis sendiri (ini yang membuktikan "engine sendiri")
| Subsistem | Keterangan |
|---|---|
| Game loop | Fixed timestep untuk update, render terpisah |
| Scene/State manager | Menu, Gameplay, Menang, Kalah, Pause |
| Renderer 2D | Memuat tekstur, sprite, sprite batch, atlas, urutan gambar (layer) |
| Input system | Keyboard + mouse, state per frame (down/pressed/released) |
| Kamera 2D | Posisi, zoom, konversi koordinat layar↔dunia |
| Collision detection | AABB / lingkaran / sweep sederhana, spatial grid jika perlu |
| Entity/object system | Entitas, update, lifecycle |
| AI | State machine & steering sederhana (satwa, Wowo, Tedduy) |
| UI system | Tombol, label, bar (air, waktu), layout |
| Audio | Pemutaran SFX/BGM, volume; decoding boleh pakai library jika disetujui |
| Particle | Api dan percikan air |

### Boleh (level rendah)
Library **bukan engine** yang hanya menyediakan akses perangkat keras.

**Sudah dikonfirmasi tetap dihitung engine buatan sendiri (06/11/2026):**
- Binding OpenGL (mis. OpenTK/Silk.NET untuk C#, GLAD/GLFW untuk C++)
- Windowing & konteks grafis

**Belum dikonfirmasi eksplisit, tanya tim dulu:**
- Decoder gambar (mis. stb_image / StbImageSharp)
- Decoder audio / akses perangkat audio (mis. OpenAL)

Aturan praktis: jika library itu sudah menyediakan game loop, scene, sprite batch, collision, atau entity system, itu **engine/framework** dan dilarang. Jika hanya memberi "jendela, konteks GPU, dan file loader", itu kandidat boleh.

### Wajib
1. **Tanya dulu** sebelum menambah dependency baru (NuGet, vcpkg, npm, dll.). Catat dependency yang disetujui di `06-architecture.md` bagian "Dependency yang disetujui".
2. **Pisahkan Engine dan Game** (project/namespace terpisah). Engine tidak boleh bergantung pada kode Game.
3. Tulis kode yang bisa dijelaskan: tim akan diuji soal cara kerja engine, dan penggunaan AI wajib diungkap sesuai *Disclaimer Penggunaan AI* di laporan.
4. Jaga riwayat commit yang rapi dan bertahap sebagai bukti pengembangan sendiri.
5. Dokumentasikan arsitektur engine (diagram + penjelasan modul) untuk laporan akhir.

## Checklist bukti "engine sendiri" (untuk laporan & presentasi)
- [ ] Diagram arsitektur engine (modul & dependensi)
- [ ] Daftar subsistem yang dibuat sendiri + penjelasan singkat algoritmanya
- [ ] Daftar dependency level rendah + alasan + bukti persetujuan dosen (binding OpenGL & windowing sudah dikonfirmasi 06/11/2026)
- [ ] Riwayat commit bertahap per tahap implementasi
- [ ] Demo: engine bisa menjalankan scene lain di luar Burn The Wo (opsional, nilai plus)

## Status konfirmasi ke dosen
1. Binding OpenGL/windowing (mis. OpenTK/Silk.NET/GLFW) tetap dihitung "engine buatan sendiri": **DIKONFIRMASI** (06/11/2026).
2. Library decoder gambar/audio: _belum ditanyakan_. Selama belum, tulis loader sederhana sendiri (mis. WAV/PNG) atau tanya tim.
3. Rubrik penilaian lain (selain plafon): _belum ada_. Jika ada, taruh di `docs/` agar bisa dijadikan acuan.
