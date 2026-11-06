# 06 — Architecture

> Dokumen hidup. Isi bagian di bawah sesuai kondisi repo yang sebenarnya. Claude Code: jangan menebak stack; baca repo dan tanya bila tidak jelas.
>
> **Snapshot kondisi repo: 06/11/2026 (akhir Tahap 1).** Solution berisi 2 proyek: `Engine` (static library) dan `Burn The Wok` (game, `.exe`). Dependency: SDL2 + GLEW di `External/`. Build Debug & Release sukses tanpa warning; demo Tahap 1 sudah dijalankan.

## Tech Stack

| Aspek | Nilai |
|---|---|
| Bahasa | **C++20** (`LanguageStandard = stdcpp20`) |
| Compiler | **MSVC**, platform toolset **v145** (di mesin pengembang: MSVC `14.51.36231`) |
| IDE | **Visual Studio Community 2026** (18.x; di mesin pengembang `18.11.12328.437`) |
| Windows SDK | `10.0` (SDK 10 terbaru yang terpasang; di mesin pengembang `10.0.26100.0`) |
| Build system | **MSBuild** (`.vcxproj` + solution `.slnx`) |
| Konfigurasi | `Debug` / `Release` × **`Win32` saja** (di `.slnx` disebut `x86`). x64 dihapus karena library contoh dosen hanya x86 |
| Grafis | **OpenGL 3.3 core profile** (shader GLSL 330) |
| Window/input | SDL2 (core) |
| Jenis output | `Burn The Wok.exe`. Debug: **SubSystem Console** (ada jendela log). Release: **SubSystem Windows** (tanpa konsol, entry point tetap `main` via `mainCRTStartup`) |
| Opsi compiler lain | Warning Level 3, SDL checks, `/permissive-`, Unicode; Release: Whole Program Optimization |
| Target OS | Windows (PC) |
| Package manager | Tidak ada; dependency di-vendor di `External/` |

### Build

Lewat IDE: buka `Burn The Wok.slnx` → pilih `Debug | x86` → **Build > Build Solution** (Ctrl+Shift+B). Project startup: `Burn The Wok` (klik kanan → *Set as Startup Project* jika VS memilih `Engine`).

Lewat command line (Developer PowerShell for VS 2026, di root repo):

```powershell
msbuild "Burn The Wok.slnx" /p:Configuration=Debug /p:Platform=x86
msbuild "Burn The Wok.slnx" /p:Configuration=Release /p:Platform=x86
```

Atau tanpa Developer PowerShell:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" "Burn The Wok.slnx" /p:Configuration=Debug /p:Platform=x86
```

Diverifikasi 06/11/2026: kedua konfigurasi sukses, 0 warning.

### Menjalankan gim

- IDE: F5 (debug) / Ctrl+F5. Working directory saat debug = folder `Burn The Wok/` (penting untuk path aset nanti).
- Manual: `Debug\Burn The Wok.exe` atau `Release\Burn The Wok.exe` (relatif ke root repo). `SDL2.dll` dan `glew32.dll` otomatis disalin ke folder itu saat build (target `CopyEngineRuntimeDlls` di `Engine/Engine.props`).
- Kontrol demo Tahap 1: Enter/Spasi = mulai, WASD/panah = gerak, roda mouse = zoom, F1/F2 = layar menang/kalah (debug), Esc = kembali ke menu / keluar dari menu.

## Dependency yang disetujui

Hanya library **non-engine** yang disetujui tim/dosen. Tambahan baru harus ditanyakan dulu (lihat `02-grading-constraints.md`).

Kategori: **(a)** dikonfirmasi dosen (binding OpenGL & windowing) · **(b)** belum dikonfirmasi (mis. decoder gambar/audio) · **(c)** game engine/framework pihak ke-3 — **MELANGGAR aturan**.

| Library | Versi | Fungsi | Kategori | Disetujui oleh/tgl |
|---|---|---|---|---|
| SDL2 (core saja) | header 2.0.3, DLL 2.0.4 | Window, konteks OpenGL, event keyboard/mouse | **(a)** windowing | Kategori dikonfirmasi dosen 06/11/2026; dipilih tim 06/11/2026 (sama dengan contoh dosen) |
| GLEW | 2.1.0 | Loader fungsi OpenGL | **(a)** binding OpenGL | idem |
| `opengl32.lib` | Windows SDK | Driver OpenGL | **(a)** | idem |

**Tidak ada dependency kategori (c).** Dari SDL2 hanya dipakai: `SDL_Init`, window + konteks GL, event, keyboard/mouse state, timer, message box. **Tidak** memakai `SDL_Renderer`, `SDL_Surface` untuk menggambar, atau audio SDL (belum).

Lokasi: `External/SDL/`, `External/GLEW/` (detail & lisensi: `External/README.md`). Hanya kode Engine yang meng-include header SDL/GLEW; kode Game tidak bisa (include path-nya tidak diberikan ke proyek Game).

### Rencana (belum ditambahkan)

| Kebutuhan | Rencana | Status |
|---|---|---|
| Decode PNG | SOIL (dipakai contoh dosen) atau stb_image | **(b)** — tanya dosen dulu. Sementara tekstur dibuat di kode (`PlaceholderArt`) |
| Audio | Perangkat audio SDL2 core + **mixer & loader WAV buatan sendiri**. Tanpa FMOD, tanpa SDL_mixer | Tahap 6 |

### Referensi dari dosen: `code-master` (dianalisis 06/11/2026)

Kode buku *Game Programming in C++* (Sanjay Madhav), lisensi BSD-3. Stack-nya:

| Library (versi) | Dipakai di | Fungsi | Kategori untuk proyek kita |
|---|---|---|---|
| SDL2 core | Semua bab | Window, konteks OpenGL, input | **(a)** — dipakai, hanya untuk window/konteks/input. `SDL_Renderer` (Bab 1–4) **tidak** dipakai |
| GLEW | Bab 5–14 | Loader OpenGL | **(a)** — dipakai |
| SDL2_image | Bab 2–4 | Decode PNG | (b) |
| SOIL | Bab 5–14 | Decode PNG → tekstur GL | (b) |
| SDL2_ttf | Bab 11–14 | Render font TrueType | (b) |
| SDL2_mixer | Di-link, tidak pernah dipanggil | Mixer + decoder audio | (b), berisiko (menyediakan mixer) — tidak dipakai |
| FMOD Studio 1.09 | Bab 7–14 | Middleware audio | Hindari (audio engine proprietary) — tidak dipakai |
| rapidjson | Bab 7–14 | Parser JSON | (b) — belum ditanyakan |

**Kode bab buku tidak disalin** ke repo ini (aturan 4 di `CLAUDE.md`). Yang diambil hanya file library dari `External/`.

## Struktur solution

```
Burn The Wok/                         # root repo (git, branch main)
├── Burn The Wok.slnx                 # solution: Engine + Burn The Wok, platform x86
├── External/                         # dependency pihak ke-3 (vendored) + README lisensi
│   ├── SDL/   include/SDL/*.h, lib/win/x86/SDL2.{lib,dll}
│   └── GLEW/  include/GL/*.h,  lib/win/x86/glew32.{lib,dll}
├── Engine/                           # Static library: GENERIK, tidak tahu apa-apa soal Burn The Wo
│   ├── Engine.vcxproj
│   ├── Engine.props                  # di-import proyek game: include path, link SDL2/GLEW/GL, salin DLL
│   ├── include/engine/               # header publik  -> #include <engine/...>
│   │   ├── core/      Application, Scene, SceneManager, Log
│   │   ├── graphics/  Window, Shader, Texture, Image, SpriteBatch, Camera2D, Color
│   │   ├── input/     Input, Keys
│   │   └── math/      Vec2, Rect, Mat4
│   └── src/                          # implementasi .cpp (struktur sama) + GL.h (internal)
├── Burn The Wok/                     # Proyek Game (.exe): referensi Engine
│   ├── Burn The Wok.vcxproj
│   └── src/
│       ├── main.cpp                  # buat Application, jalankan MenuScene
│       ├── GameConfig.h              # konstanta tuning (placeholder)
│       ├── art/       PlaceholderArt # atlas sprite yang digambar lewat kode
│       ├── entities/  Player
│       ├── world/     ForestMap      # peta tile placeholder
│       └── scenes/    MenuScene, GameplayScene, ResultScene (Menang/Kalah)
├── docs/
├── .gitignore
└── CLAUDE.md
```

Folder yang akan ditambah sesuai tahap: `engine/physics` (T2), `engine/ai` (T4), `engine/ui`, `engine/audio`, `engine/particles` (T6); di Game: `systems/` (T2–T5), `config/` data stage (T5), `assets/` (saat aset tersedia).

Aturan dependensi: `Game → Engine`. Dilarang `Engine → Game`. Ditegakkan oleh build: proyek Engine tidak punya include path ke folder Game.

## Modul Engine (Tahap 1)

| Modul | Isi | Cara kerja singkat |
|---|---|---|
| `core/Application` | Pemilik semua subsistem + game loop | Fixed timestep 60 Hz dengan accumulator; frame time dibatasi 0,25 s (anti "spiral of death"); render sekali per frame dengan `alpha` untuk interpolasi |
| `core/Scene`, `SceneManager` | State gim | Stack scene; update hanya scene teratas, draw semua dari bawah ke atas (siap untuk overlay Pause). Push/Pop/SwitchTo ditunda sampai selesai update agar scene tidak terhapus saat fungsinya masih berjalan |
| `graphics/Window` | Jendela + konteks OpenGL 3.3 core | SDL2 membuat jendela/konteks, GLEW memuat fungsi GL, vsync on |
| `graphics/Image`, `Texture` | Piksel CPU → tekstur GPU | Filter NEAREST, clamp to edge |
| `graphics/Shader` | Program GLSL | Kompilasi + link, error dilempar sebagai exception beserta log |
| `graphics/SpriteBatch` | Renderer 2D | Kumpulkan quad (4 vertex/6 index) ke satu buffer; 1 draw call per rangkaian sprite dengan tekstur yang sama. Urutan gambar = urutan panggilan |
| `graphics/Camera2D` | Kamera 2D | Posisi = titik tengah layar, zoom = piksel layar per unit dunia; proyeksi ortografis y-ke-bawah; `ScreenToWorld`/`WorldToScreen`; pixel snapping |
| `input/Input` | Keyboard & mouse | Down/Pressed/Released. "Previous" diperbarui per **fixed update**, bukan per frame, sehingga tombol tidak terbaca dua kali/terlewat |
| `math/*` | Vec2, Rect, Mat4 | Header-only |

## Prinsip desain engine

- **Game loop:** fixed timestep untuk update (60 Hz), render tiap frame; delta time konsisten.
- **Sederhana dulu:** pakai pendekatan yang cukup untuk gim 2D kecil; hindari over-engineering (mis. ECS penuh) kecuali terbukti perlu.
- **Data-driven:** stage dan parameter balancing di file konfigurasi, bukan hard-coded.
- **Pixel-perfect:** filter tekstur *nearest*, zoom bilangan bulat, posisi render dibulatkan ke piksel dunia, kamera di-snap ke piksel layar.
- **Terpisah & dapat diuji:** logika (api, air, skor) dipisah dari rendering agar mudah diuji.

## Konvensi kode

> **Disetujui tim 06/11/2026.** Dari konfigurasi proyek: C++20, Unicode, `/permissive-`, Warning Level 3.

- Penamaan:
  - Tipe (class/struct/enum): `PascalCase` — `SpriteBatch`, `FireSystem`
  - Fungsi/method: `PascalCase` — `Update()`, `Draw()`; accessor pakai `Get`/`Set`/`Is` — `GetWidth()`, `IsKeyDown()`
  - Variabel lokal & parameter: `camelCase` — `deltaTime`
  - Member data: prefix `m_` — `m_position`
  - Pengecualian: struct data sederhana yang field-nya publik (`Vec2`, `Rect`, `Color`) tanpa `m_` — `v.x`, `rect.w`
  - Konstanta & enumerator: `PascalCase` (`enum class`) — `Key::Escape`
  - Namespace: huruf kecil — `engine`, `game`
  - File: sama dengan nama tipe utamanya — `SpriteBatch.h` / `SpriteBatch.cpp`
- Include: header Engine dengan `<engine/...>` dari Game, `"engine/..."` di dalam Engine; header Game dengan `"folder/File.h"`.
- Header: `#pragma once`.
- Memori: RAII, `std::unique_ptr` untuk kepemilikan; hindari `new`/`delete` manual. Objek yang memegang resource GPU tidak bisa di-copy.
- Error fatal saat inisialisasi: lempar `std::runtime_error`; `main` menangkap dan menampilkan message box.
- Source hanya ASCII (MSVC membaca file tanpa BOM sebagai codepage lokal).
- Komentar: Inggris, jelaskan *kenapa*, bukan *apa*
- Satu commit = satu perubahan logis; pesan commit jelas

## Alur data utama

```
SDL events → Input ─┐
                    ├→ SceneManager.Update(1/60 s) → Scene → [Player, Map, ... (nanti: Systems)]
Application loop ───┤
                    └→ SceneManager.Draw(alpha) → Scene → SpriteBatch(Camera2D) → OpenGL → Window.Present
```

## Catatan teknis / keputusan arsitektur

- 06/11/2026 — Stack: C++20 + MSVC v145 + MSBuild (VS 2026), Win32, OpenGL 3.3 core via SDL2 + GLEW.
- 06/11/2026 — `SDL_MAIN_HANDLED` + `SDL_SetMainReady()`: memakai `main()` biasa, sehingga `SDL2main.lib` tidak diperlukan dan Release bisa memakai SubSystem Windows dengan entry point `mainCRTStartup`.
- 06/11/2026 — Tekstur Tahap 1 dibuat lewat kode (`Image` → `Texture`), karena decoder PNG belum disetujui. Nanti loader PNG cukup menghasilkan `engine::Image`; sisa pipeline tidak berubah.
- 06/11/2026 — Ukuran tile placeholder 16×16 px, zoom default 3× (`GameConfig.h`). Belum final; menunggu aset.
- 06/11/2026 — Belum ada collision: pemain bisa berjalan menembus pohon/air (Tahap 2).

_(Tambahkan di sini seiring pengembangan. Salin ringkasannya ke laporan bab arsitektur.)_
