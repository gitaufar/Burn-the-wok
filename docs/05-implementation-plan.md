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

- [x] Entity/object system sederhana _(13/11, branch `feat/stage2-fire-water`)_
- [x] Tilemap (pohon, tanah, sumber air) _(13/11)_
- [x] Collision detection (AABB/lingkaran) + query area _(09/11, branch `feat/add-collision`)_
- [x] Objek Api (intensitas/HP) _(13/11)_
- [x] Penyemprot air: arah dari mouse, partikel/semprotan, collision dengan api _(13/11)_
- [x] Sistem air: kapasitas, berkurang saat menyemprot, isi ulang di sumber air _(13/11)_

**DoD:** pemain bisa memadamkan api dan mengisi ulang air. **Kode lengkap 13/11/2026; menunggu uji main manual oleh tim** (lihat catatan di bawah).

Collision 09/11/2026: modul Engine baru `engine/physics/Collision` (generik): tes overlap AABB–AABB, lingkaran–lingkaran, lingkaran–AABB; `TilesOverlapping` / `OverlapsSolidTile` untuk query area di grid tile; `MoveAndSlide` untuk gerak kotak di grid tile (sumbu X lalu Y, berhenti rapat di tile padat dan meluncur di sepanjang dinding, setiap kolom/baris yang dilewati diperiksa sehingga tidak bisa menembus). Di Game: pohon dan air padat (`ForestMap::IsSolid`), pemain punya hitbox kaki 10×6 px, tombol **F3** menampilkan hitbox dan tile padat di sekitar pemain. Query siap pakai untuk anggota tim lain: `ForestMap::IsBlocked(area)`, `ForestMap::HasTileInArea(area, TileType::Water)` (deteksi di dekat kolam: perbesar hitbox pemain beberapa piksel), `engine::Overlaps(Circle, Rect)` untuk semprotan vs api. Verifikasi: build Debug & Release 0 warning; uji unit fungsi collision (21 pengecekan) dan simulasi 200.000 langkah acak di peta asli — pemain tidak pernah masuk tile padat; gim dijalankan tanpa crash. **Belum diuji manual:** rasa gerak saat menabrak pohon/kolam — mohon dicoba (F3 untuk melihat hitbox).

Api & air 13/11/2026 (branch `feat/stage2-fire-water`):
- **Engine** — modul baru `engine/entity`: `Entity` (posisi + posisi sebelumnya untuk interpolasi, flag hidup, `Update`/`Draw` virtual) dan `EntityWorld` (spawn, update, draw, `ForEach<T>`, `Count<T>`, hapus entity mati). Spawn di tengah `Update`/`ForEach` ditunda sampai iterasi selesai, sehingga aman. Generik, tidak tahu soal api/air.
- **Tilemap** — jalur tanah kini tile sungguhan `TileType::Dirt` (sebelumnya hanya hiasan gambar). Tile: rumput, rumput berbunga, tanah (bisa dilewati), pohon, air (padat).
- **Api** (`entities/Fire`) — menempel di satu tile pohon; intensitas = HP (mulai 60, maks 100). Jika tidak disemprot selama 1 detik, api tumbuh lagi 6/detik. Area kena = lingkaran yang mengecil seiring intensitas. Saat mulai ada 6 api di pohon acak yang bisa didekati (seed tetap). Api belum menyebar (Tahap 3).
- **Penyemprot** (`systems/Sprayer`, `entities/WaterDrop`) — tahan klik kiri: 30 tetes/detik ke arah kursor dengan sebaran acak ±7°, jangkauan sekitar 3 tile. Tetes melewati pohon karena apinya ada di pohon. Pemain menghadap ke arah semprotan.
- **Tumbukan air–api** (`systems/FireSystem`) — tetes yang mengenai lingkaran api hilang dan mengurangi intensitas 2,5. Api yang padam hilang dan mengeluarkan asap (`entities/Particle`), ada uap kecil di titik kena, dan api tampak pucat sesaat ketika terkena.
- **Sistem air** (`systems/WaterTank`, logika murni) — kapasitas 100, satu tetes = 0,4 (sekitar 8 detik semprot penuh). Isi ulang 45/detik saat hitbox pemain (diperbesar 4 px) menyentuh tile air.
- **HUD** — bar AIR + persen, status (KLIK KIRI: SEMPROT / MENGISI ULANG / AIR HABIS - KE KOLAM), jumlah api & yang sudah padam, titik api berkedip di minimap, petunjuk mouse di panel kontrol.
- **Debug** — F4 membakar pohon di bawah kursor.
- **Aset** — sprite api (3 frame) dan cahaya dibuat lewat kode di `PlaceholderArt` karena belum ada aset PNG. Semua angka di `GameConfig.h`.
- **Verifikasi**: build Debug & Release 0 warning. Program uji headless (entity world, tangki, laju semprot, aturan nyala api, memadamkan api dari jarak 3 tile vs 8 tile, isi ulang di tepi kolam vs di tengah peta): **108 pengecekan lolos** (13/11, setelah Smart App Control di laptop pengembang dimatikan; sebelumnya `.exe` hasil build diblokir). **Belum diuji manual:** rasa menyemprot, keterbacaan api di balik pohon, HUD di berbagai ukuran jendela — mohon dicoba (`.\run`).
- Ikut diperbaiki: build Release kembali bisa di-link. Setting `/MT` Engine Release terhapus di commit `b1049e7`, sehingga Engine (`/MD`) bentrok dengan Game (`/MT`).

## Tahap 3 — Penyebaran api & perubahan lahan (13/11)

- [x] Api menyebar ke pohon sekitar _(13/11, branch `feat/stage2-fire-water`)_
- [x] State pohon: sehat → terbakar → (jika dibiarkan) sawit; hangus = bekas gosong visual saja (lihat catatan) _(13/11)_
- [x] Timer stage _(13/11)_
- [x] Persentase hutan tersisa _(13/11)_
- [x] Kondisi menang/kalah dasar (batas sawit, api padam) _(13/11)_

**DoD:** loop inti jalan: api menyebar, pohon berubah jadi sawit, gim bisa menang atau kalah. **Kode lengkap 13/11/2026, lolos uji simulasi; menunggu uji main manual oleh tim.**

Penyebaran & sawit 13/11/2026 (branch yang sama dengan sisa Tahap 2):
- **Penyebaran** (`FireSystem::Update`) — api yang intensitasnya ≥50% menyalakan satu pohon acak dalam radius 2 tile setiap 12 detik. Api yang disemprot sampai di bawah 50% berhenti menyebar dan juga terbakar lebih lambat. Pohon di tepi peta (border) tidak ikut terbakar (pemandangan/batas peta).
- **State pohon** — sehat → terbakar (pohon makin gelap/gosong selama terbakar) → setelah 30 detik pada intensitas penuh (sampai 60 detik bila lemah) menjadi tile **`Sawit`**: pohon sawit di atas lahan tanah, padat, tidak bisa terbakar lagi, dan tidak dihitung sebagai hutan. Pohon yang dipadamkan tetap hutan dengan bekas gosong. "Hangus" sebagai mekanik (lahan untuk bibit sawit) ditunda ke Tahap 5.
- **Aturan stage** (`systems/StageRules`, logika murni) — timer 150 detik. Menang: semua api padam sebelum waktu habis. Kalah: sawit ≥15% dari pohon awal (152 pohon di peta sekarang → 23 pohon), atau waktu habis saat api masih menyala. Kalah karena sawit diperiksa lebih dulu daripada "api habis". Setelah hasil ditentukan, ada jeda 1,5 detik dengan banner di layar, lalu pindah ke layar hasil.
- **HUD** — panel tengah atas: sisa waktu (berkedip merah ≤30 detik), % hutan, bar sawit menuju batas kalah. Minimap ikut berubah saat pohon jadi sawit (kuning).
- **Layar hasil** — judul MENANG/KALAH, alasan, % hutan tersisa, % sawit, jumlah api dipadamkan, sisa waktu. F1/F2 (debug) tetap ada tanpa statistik.
- **Font pixel** — ditambah simbol `%`, `/`, `.` (sebelumnya "/" di "WASD / PANAH" tidak tergambar).
- **Aset** — sprite sawit dibuat lewat kode (`BuildSawitImage`), placeholder.
- **Verifikasi**: build Debug & Release 0 warning; gim berhasil dibuka sampai menu. Uji headless **71 pengecekan lolos**: aturan menang/kalah, tile/gosong, pohon jadi sawit setelah ~30,7 detik, penyebaran pertama tepat di 12,0 detik, api lemah tidak menyebar, satu pohon tidak pernah punya dua api. Simulasi stage penuh: **tanpa pemain selalu kalah** karena sawit (detik 67–91, 3 seed); **bot sederhana selalu menang** (detik 27–44, 5 seed). Bot terbang lurus tanpa terhalang pohon, jadi pemain sungguhan lebih lambat. Angka balancing masih placeholder (Tahap 7).
- **Belum diuji manual:** rasa tekanan waktu, keterbacaan sawit & pohon gosong, panel waktu/banner, layar hasil — mohon dicoba (`.\run`).

Aset asli 13/11/2026: sprite api & sawit placeholder (gambar kode) diganti `items/fire.png` dan `items/palm_oil_tree.png`; penyemprot (`items/sprayer_*`) dan semburan air (`items/water_spray_*`) ditambahkan (detail di `04-characters-assets.md`). Nosel semprotan dipindah ke ujung penyemprot (12 px dari pemain). Dicek lewat screenshot gim: api, sawit, penyemprot kiri/kanan, dan semburan tampil benar. Uji headless 108 + 71 tetap lolos.

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

- Tahap aktif: **2 & 3 kode lengkap** (13/11/2026), menunggu uji main manual; berikutnya **Tahap 4** (jadwal 16/11)
- Terakhir diperbarui: 13/11/2026
- Kondisi repo (13/11/2026): solution `Engine` (static lib) + `Burn The Wok` (exe), C++20/MSVC v145/Win32, SDL2 + GLEW di `External/`. Remote GitHub `gitaufar/Burn-the-wok`, kerja lewat branch + PR. Aset PNG: pohon, logo, sprite pemain; sisanya placeholder lewat kode. Jalankan dengan `.\run` (lihat `06-architecture.md`).
- Status per tahap:

| Tahap | Status |
|---|---|
| 1 — Engine core | **Selesai** (06/11/2026) |
| 2 — Collision & air/api | **Kode lengkap** (13/11/2026), menunggu uji main manual |
| 3 — Penyebaran api & lahan | **Kode lengkap** (13/11/2026), menunggu uji main manual |
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
| 06/11/2026 | Distribusi = zip Windows (`tools/package-release.bat`), dikirim via Google Drive atau itch.io (downloadable). Release memakai runtime statis `/MT`. Tidak ada deploy web (Vercel dsb.) | Gim native Windows; dosen cukup ekstrak & jalankan tanpa install apa pun |
| 06/11/2026 | Dependency pihak ke-3 di-vendor di repo, folder `External/` | Paling mudah untuk kolaborasi (clone → build tanpa instal); sama dengan pola contoh dosen |
| 06/11/2026 | Repo git dibuat (branch `main`) + `.gitignore`; remote menyusul | Bukti commit bertahap (lihat 02) |
| 06/11/2026 | Konvensi kode di `06-architecture.md` disetujui | Keputusan tim |
| 09/11/2026 | Pembagian Tahap 2: collision dikerjakan di branch `feat/add-collision`; item Tahap 2 lainnya (entity, api, penyemprot, sistem air) dikerjakan anggota tim lain | Bagi tugas tim |
| 09/11/2026 | Pohon dan air = tile padat; hanya kaki pemain (hitbox 10×6 px) yang bertabrakan. "Isi ulang di sumber air" ditafsirkan sebagai *berdiri di tepi* kolam (cek dengan `HasTileInArea`), bisa diubah tim | Air padat mencegah pemain berjalan di atas kolam; hitbox kaki terasa wajar untuk tampilan top-down |
| 13/11/2026 | Sisa Tahap 2 (entity, api, penyemprot, sistem air) dikerjakan dengan bantuan Claude Code di branch `feat/stage2-fire-water` | Permintaan tim; Tahap 2 terlambat dari jadwal 09/11 |
| 13/11/2026 | Entity system = kelas dasar `Entity` + `EntityWorld` sederhana (bukan ECS). Interaksi antar-entity ada di *system* Game (`FireSystem`, `Sprayer`), bukan di entity | Cukup untuk gim 2D kecil (lihat prinsip "sederhana dulu" di 06); entity tidak saling bergantung |
| 13/11/2026 | Api = HP (intensitas) yang tumbuh lagi jika tidak disemprot; tetes air melewati pohon; isi ulang otomatis di tepi kolam (tanpa tombol) | Usulan; angka di `GameConfig.h`, bisa diubah saat balancing (Tahap 7) |
| 13/11/2026 | Pemain (`Player`) belum dijadikan `Entity`; tetap objek khusus di scene | Pemain butuh input & peta di `Update`; dipindah jika Tahap 4–5 butuh (mis. musuh menargetkan pemain) |
| 13/11/2026 | Sprite api/cahaya dibuat lewat kode (placeholder); plugin SpriteCook belum bisa dipakai karena belum login | Ganti dengan PNG cukup di `PlaceholderArt` / `Fire::Draw` |
| 13/11/2026 | Tahap 3 dikerjakan di branch yang sama (`feat/stage2-fire-water`) | Permintaan tim, langsung setelah sisa Tahap 2 |
| 13/11/2026 | Menang = semua api padam sebelum waktu habis (langsung, tidak menunggu timer). Kalah = sawit ≥ batas, **atau waktu habis saat api masih menyala**. Syarat satwa menyusul di Tahap 4 | Tafsiran 03-gameplay-design ("sebelum waktu habis"); "waktu habis = kalah" tidak tertulis eksplisit, mohon dikonfirmasi tim |
| 13/11/2026 | Persentase hutan & sawit dihitung terhadap pohon awal **tanpa** pohon tepi peta (border); pohon border tidak bisa terbakar | Border = batas peta yang tidak bisa didekati dari luar; menghindari api yang tidak terjangkau |
| 13/11/2026 | Pohon yang dipadamkan tetap dihitung hutan (hanya bekas gosong visual). "Lahan hangus" sebagai state gameplay ditunda ke Tahap 5 | 05 menyebut "hangus jika perlu"; baru dibutuhkan saat anak buah menanam bibit sawit di lahan hangus |
| 13/11/2026 | Penyebaran = loncat ke pohon acak dalam radius 2 tile (bukan hanya tetangga langsung) | Pohon di peta jarang (±12%); dengan tetangga langsung api hampir tidak pernah menyebar |
| 13/11/2026 | Angka awal Tahap 3: timer 150 s, sawit kalah 15%, jeda sebar 12 s (hanya api ≥50%), pohon → sawit 30 s | Placeholder di `GameConfig.h`; simulasi: tanpa pemain kalah ±70–90 s, bot sederhana menang ±30–45 s |
| 13/11/2026 | Aset item dari tim aset menggantikan placeholder api & sawit; animasi api dari satu gambar (cermin + goyang), bukan frame gambar tangan | Aset hanya 1 gambar api; ganti dengan animasi berframe jika tim aset membuatnya |
| 06/11/2026 | Contoh dosen = kode buku *Game Programming in C++* (Madhav); dipakai sebagai referensi, **kodenya tidak disalin** | Aturan engine sendiri |
| 09/11/2026 | Revisi HUD eksplorasi: judul lokasi terbuka dengan lencana, peta kertas lipat dengan kompas utara dan margin koordinat, tombol WASD tersusun spasial | Permintaan perbaikan visual halaman gameplay; memakai renderer pixel internal tanpa dependency baru |
| 09/11/2026 | Menu start diselaraskan dengan HUD: logo asli dipertahankan, lencana kawasan, catatan kontrol kertas, tombol mulai warna krem dengan hover, font pixel bersama | Revisi visual permintaan pengguna; kontrol Enter/Spasi/klik dan Esc tetap berlaku |
| 13/11/2026 | Audio awal (39 SFX WAV mono + 3 musik OGG + 1 suara) diambil dari sumber CC0 dan dicatat di `assets/audio/CREDITS.md`. Musik disimpan sebagai OGG karena WAV stereo 15 MB+ per lagu; pemutarnya butuh decoder OGG (kategori b, **tanya dosen dulu**, mis. stb_vorbis atau decoder buatan sendiri) atau konversi ke WAV mono/22 kHz saat build | Ukuran repo dan zip rilis |
