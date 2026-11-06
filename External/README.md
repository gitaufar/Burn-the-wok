# External (dependency pihak ke-3)

Library level rendah yang **bukan engine**. Semua disalin apa adanya dari contoh dosen
(`code-master/External`, kode buku *Game Programming in C++*, Sanjay Madhav) pada 06/11/2026.
Hanya binary Windows **x86 (32-bit)** yang tersedia, karena itu proyek hanya punya konfigurasi `Win32`.

| Folder | Library | Versi | Dipakai untuk | Lisensi |
|---|---|---|---|---|
| `SDL/` | SDL2 (core saja) | header 2.0.3, DLL 2.0.4 | Window, konteks OpenGL, event keyboard/mouse | zlib (`SDL/COPYING.txt`) |
| `GLEW/` | GLEW | 2.1.0 | Loader fungsi OpenGL | Modified BSD/MIT (`GLEW/LICENSE.txt`) |

Yang sengaja **tidak** disalin dari contoh dosen: SDL2_image, SDL2_mixer, SDL2_ttf, SOIL, rapidjson, FMOD,
header `SDL_test*`, dan `SDL2main.lib` (kita memakai `SDL_MAIN_HANDLED`, jadi `main()` biasa).

Aturan: jangan menambah library baru di sini tanpa persetujuan tim (lihat `docs/02-grading-constraints.md`),
dan catat setiap tambahan di `docs/06-architecture.md` bagian "Dependency yang disetujui".
