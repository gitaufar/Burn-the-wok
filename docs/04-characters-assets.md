# 04 — Characters & Assets

## Gaya visual

- **Pixel art 2D** untuk semua karakter dan objek, bernuansa retro.
- **Lingkungan:** hutan tropis Indonesia, kawasan terbakar, dan perkebunan sawit.
- **Palet warna:** hijau alami, oranye, merah, dan kuning api untuk membangun kontras visual (hijau hutan vs api vs sawit).
- **Visual tone:** suasana dramatis dengan humor satir sebagai kritik sosial.
- **Logo/visual identity:** teks "BURN THE WO" bergaya pixel dengan api; perpaduan estetika retro, hutan tropis, dan karakter satiris.

## Karakter

Semua karakter memiliki sprite **4 arah: depan, belakang, kiri, kanan**.

| Karakter | Peran | Catatan |
|---|---|---|
| **Wowo** | Karakter utama / figur pemimpin satiris | Antagonis (AI) sabotase; turun langsung menghadang pemain di stage 3 |
| **Tedduy** | Karakter utama / pendamping Wowo | Antagonis sabotase di stage 2–3 (muncul langsung di stage 3) |
| **Orang Utan** | Satwa yang harus diselamatkan | Muncul sejak stage 1 |
| **Bekantan** | Satwa yang harus diselamatkan | Muncul sejak stage 2 |
| **Burung Hantu** | Satwa yang harus diselamatkan | Muncul di stage 3 |
| **Penjaga hutan (pemain)** | Karakter yang dikendalikan pemain; membawa penyemprot air | Dikonfirmasi tim: pemain = penjaga hutan, terpisah dari Wowo/Tedduy. Sprite 4 arah perlu dibuat |
| **Anak buah Wowo/Tedduy** | Penyebar api & penanam sawit (stage 2) | _Aset belum ada di laporan_ |

## Item & objek lingkungan

| Aset | Fungsi |
|---|---|
| Penyemprot air | Alat pemain untuk memadamkan api |
| Air | Efek semprotan / sumber daya |
| Api | Bahaya utama; menyebar & merusak pohon |
| Pohon | Objek hutan yang harus dijaga |
| Pohon sawit | Hasil perubahan pohon terbakar yang tidak dipadamkan; indikator kekalahan |
| Sumber air | Titik isi ulang _(perlu aset)_ |

## Aset yang sudah ada di repo (dicek 06/11/2026)

**Belum ada aset sama sekali di repo.** Penelusuran seluruh folder tidak menemukan file gambar (`.png`, `.bmp`, `.gif`, `.jpg`, `.aseprite`, `.psd`), audio (`.wav`, `.ogg`, `.mp3`), maupun font (`.ttf`, `.otf`, bitmap font). Folder `Assets/` belum dibuat.

| Kategori | File | Format | Ukuran | Lokasi |
|---|---|---|---|---|
| Sprite | _(tidak ada)_ | — | — | — |
| Audio | _(tidak ada)_ | — | — | — |
| Font | _(tidak ada)_ | — | — | — |

- Ukuran tile terdeteksi: **BELUM DIKETAHUI** (tidak ada aset untuk diukur)
- Ukuran sprite terdeteksi: **BELUM DIKETAHUI**
- Laporan menyebut sprite Wowo, Tedduy, orang utan, bekantan, burung hantu, pohon, sawit, api, air, dan penyemprot. **Dikonfirmasi tim 06/11/2026: file aset belum ada sama sekali** (termasuk di luar repo). Tahap 1 memakai placeholder; ukuran tile/sprite disepakati saat aset mulai dibuat.

### Update aset di repo (13/11/2026, branch `feat/add-assets`)

Bagian di atas dicek 06/11 dan sudah usang. Isi `Burn The Wok/assets/` sekarang:

| Folder | Isi | Catatan |
|---|---|---|
| `characters/` | Penjaga hutan: 16 PNG `Front 1-4`, `Back 1-4`, `Left 1-4`, `Right 1-4` (nama file lama tidak seragam, mis. `Back 11.png`, `Right.png`; dimuat apa adanya oleh `Player::LoadArtwork`) | 80×80 px |
| `characters/<nama>/` | `bekantan`, `orangutan`, `burung_hantu`, `wowo`, `tedduy`: masing-masing 16 PNG `<arah>_<n>.png` (arah: front/back/left/right, n: 1-4) | 80×80 px, latar transparan |
| `items/` | `sprayer_right/left`, `water_spray_right/left`, `fire`, `palm_oil_tree` | Item diam; hanya penyemprot dan semprotan yang punya versi kiri-kanan |
| `environment/Tree.png`, `ui/...Emblem.png` | Pohon dan logo | dari tim aset |
| `audio/`, `fonts/` | kosong | belum ada |

Frame jalan karakter baru dibuat dari satu pose per arah: kaki kiri dan kanan bergantian terangkat secara otomatis (bukan animasi gambar tangan). Urutan frame: 1 diam, 2 kaki kiri naik, 3 diam, 4 kaki kanan naik. Ganti dengan animasi gambar tangan jika sudah tersedia. Penentuan siapa Wowo dan siapa Tedduy mengikuti dugaan sementara (setelan krem berpeci = `wowo`, seragam hijau = `tedduy`); tukar nama folder jika terbalik.

### Aset item yang sudah dipakai di gim (13/11/2026)

| File | Dipakai untuk | Cara tampil |
|---|---|---|
| `items/fire.png` | Api di pohon (`Fire::Draw`) + ikon api di HUD | Tinggi 12–26 px dunia sesuai intensitas; animasi kedip = gambar dicerminkan bergantian + tinggi bergoyang sedikit; api kuat dapat 2 api kecil di samping. Cahaya oranye di bawahnya masih dari atlas kode (`Glow`) |
| `items/palm_oil_tree.png` | Tile `Sawit` (`ForestMap::DrawTrees`) | Tinggi `SawitHeight` (0,9 × pohon), akar di dasar tile, di atas tanah |
| `items/sprayer_left/right.png` | Penyemprot di tangan penjaga hutan selama klik kiri ditahan | Tinggi 12 px dunia, sisi kiri/kanan mengikuti arah bidik; saat membidik ke atas digambar di belakang pemain |
| `items/water_spray_left/right.png` | Semburan air di ujung penyemprot | Hanya saat air keluar dan bidikan cukup ke samping (gambar hanya menghadap kiri/kanan); tetes air (`WaterDrop`, kotak 2 px) tetap ada karena tetes itulah yang mengenai api |

Semua gambar dipotong otomatis ke bagian yang tidak transparan (`Image::GetContentBounds`). Belum dipakai: sprite satwa (Tahap 4) dan Wowo/Tedduy (Tahap 5).

### Placeholder di kode (Tahap 1)

Bukan file aset: atlas 128×16 px digambar piksel demi piksel oleh `Burn The Wok/src/art/PlaceholderArt.cpp` saat gim berjalan (decoder PNG belum disetujui). Isinya 8 sprite 16×16: rumput, rumput berbunga, pohon, air, dan penjaga hutan 4 arah (bawah/atas/kiri/kanan). Ukuran tile placeholder = **16×16 px** (`GameConfig.h`), ditampilkan dengan zoom 3×. Ini hanya untuk demo; ganti dengan aset asli begitu ada.

## Spesifikasi aset teknis (usulan, sepakati di Tahap 1)

Isi dan kunci nilai-nilai ini agar tim artis dan programmer sinkron:

- Ukuran tile dunia: _TBD_ (usulan 16×16 atau 32×32 px)
- Ukuran sprite karakter: _TBD_ (usulan 32×32 px)
- Format: PNG dengan alpha (transparan), tanpa anti-aliasing (pixel art tajam; filter tekstur = nearest)
- Atlas: satu spritesheet per kategori (karakter, lingkungan, UI) atau satu atlas besar
- Konvensi penamaan: `snake_case`, mis. `wowo_front.png`, `tree_burning_01.png`
- Frame animasi: _TBD_ (api, semprotan, jalan)
- Lokasi aset di repo: `Assets/` (sesuaikan dengan struktur proyek) — **belum dibuat**; usulan di `06-architecture.md`: `Burn The Wok/assets/` (working directory debug = folder proyek)
- Audio: format SFX/BGM _TBD_ (mis. WAV untuk SFX, OGG untuk BGM — tergantung decoder yang disetujui)

## Daftar aset yang masih perlu dibuat/diambil

_Dicek 06/11/2026: belum ada item yang ditemukan di repo, jadi belum ada yang dicentang._

- [ ] Sprite pemain (4 arah, animasi jalan & menyemprot)
- [ ] Sprite anak buah Wowo/Tedduy
- [ ] Sumber air
- [ ] Animasi api (beberapa tingkat intensitas), tahap pohon terbakar & pohon hangus
- [ ] Efek semprotan air
- [ ] UI: tombol, bar air, bar waktu, ikon bintang, layar menang/kalah
- [ ] SFX: semprot, api, satwa, sabotase, UI; BGM menu & gameplay
- [ ] Lisensi/sumber tiap aset eksternal (jika ada) — catat untuk laporan

## Daftar audio (13/11/2026)

Lokasi: `Burn The Wok/assets/audio/` dengan subfolder `sfx/`, `music/`, `voice/`. Nama file `snake_case`; varian diberi akhiran `_1`, `_2`, dst. Pemutar audio (mixer buatan sendiri) baru dibuat di Tahap 6, jadi file hanya disiapkan dulu.

Prioritas: **W** = wajib, **S** = sebaiknya ada, **O** = opsional. Kolom "Sumber/lisensi" diisi tim untuk laporan.

### Dari daftar tim

| # | File | Jenis | Varian | Loop | Pri | Dipicu saat | Sumber/lisensi |
|---|---|---|---|---|---|---|---|
| 1 | `sfx/ui_click` | SFX | 2 | tidak | W | klik tombol | |
| 2 | `music/intro` | Musik | 1 | ya | W | menu utama | |
| 3 | `voice/hey_antek_aseng` | Suara | 1-2 | tidak | W | antek/Wowo muncul atau mulai sabotase | |
| 4 | `sfx/win` | Jingle | 1 | tidak | W | layar menang | |
| 5 | `sfx/lose` | Jingle | 1 | tidak | W | layar kalah | |
| 6 | `sfx/animal_saved` | SFX | 1-3 | tidak | W | satwa selamat | |
| 7 | `sfx/tree_to_palm` | SFX | 1-2 | tidak | W | pohon berubah jadi sawit | |
| 8 | `sfx/water_spray` | SFX | 1 | ya (mulai/berhenti) | W | menyemprot air | |
| 9 | `sfx/footstep` | SFX | 3-4 | tidak | W | langkah pemain (acak, selang waktu tetap) | |
| 10 | `sfx/fire_out` | SFX | 2 | tidak | W | api padam | |
| 11 | `music/sabotage_tension` | Musik | 1 | ya | W | menghadapi sabotase (stage 2-3) | |

### Usulan tambahan

| # | File | Jenis | Varian | Loop | Pri | Dipicu saat |
|---|---|---|---|---|---|---|
| 12 | `sfx/fire_loop` | SFX | 1 | ya | W | ada api di layar; volume naik sesuai jumlah api |
| 13 | `sfx/fire_spread` | SFX | 3 | tidak | W | api menyebar ke pohon baru |
| 14 | `music/gameplay` | Musik | 1 per stage (1-3) | ya | W | selama bermain; intensitas naik di stage akhir |
| 15 | `sfx/water_refill` | SFX | 1 | tidak/ya | W | isi ulang di sumber air |
| 16 | `sfx/tank_empty` | SFX | 1 | tidak | W | menyemprot saat air habis |
| 17 | `sfx/timer_warning` | SFX | 1 | ya | S | 10 detik terakhir |
| 18 | `sfx/tree_burning` | SFX | 2 | tidak | S | pohon terbakar lalu hangus (derak kayu) |
| 19 | `sfx/animal_panic_orangutan`, `_bekantan`, `_burung_hantu` | SFX | 2 per hewan | tidak | S | satwa panik |
| 20 | `sfx/animal_burned` | SFX | 1 | tidak | S | satwa terbakar |
| 21 | `sfx/saboteur_ignite` | SFX | 2 | tidak | S | antek menyebar api |
| 22 | `sfx/saboteur_plant_seed` | SFX | 2 | tidak | S | antek menanam bibit sawit |
| 23 | `sfx/boss_appear` | Stinger | 1 | tidak | S | Wowo dan Tedduy muncul (stage 3) |
| 24 | `sfx/stage_start` | Jingle | 1 | tidak | S | stage dimulai |
| 25 | `sfx/stage_clear_star` | Jingle | 3 (1, 2, 3 bintang) | tidak | S | akhir stage |
| 26 | `sfx/ui_hover` | SFX | 1 | tidak | O | kursor di atas tombol |
| 27 | `sfx/pause_in`, `sfx/pause_out` | SFX | 1 each | tidak | O | pause dan lanjut |
| 28 | `sfx/footstep_grass`, `sfx/footstep_dirt` | SFX | 3 each | tidak | O | pengganti #9 per permukaan |

### Spesifikasi teknis (usulan)

- **Format: WAV PCM 16-bit, 44,1 kHz.** SFX mono; musik stereo. Belum ada decoder OGG/MP3 yang disetujui (lihat `02-grading-constraints.md`), jadi musik juga WAV dulu.
- **Durasi:** SFX 0,1-2 dtk; jingle 2-5 dtk; musik loop 30-90 dtk. Ukuran total sebaiknya kecil agar zip rilis tidak membengkak (musik WAV stereo ±10 MB per menit).
- **Loop:** titik awal dan akhir harus menyambung tanpa klik. Potong di titik nol (zero crossing).
- **Volume:** normalisasi puncak sekitar -3 dB; musik sedikit lebih pelan dari SFX. Mixer nanti punya volume terpisah untuk musik dan efek.
- **Variasi:** putar varian secara acak (jangan varian yang sama dua kali berturut-turut) agar suara berulang tidak terdengar monoton.
- **Lisensi:** catat sumber dan lisensi tiap file (buatan sendiri, CC0, atau bebas royalti) di kolom "Sumber/lisensi". Wajib untuk laporan.

### Status audio (13/11/2026, branch `feat/add-audio`)

Folder: `Burn The Wok/assets/audio/{sfx,music,voice}/`. Sumber, pembuat, dan lisensi (semua CC0) per file: `assets/audio/CREDITS.md`. File yang berasal dari tim (bukan CC0 terverifikasi) ditandai terpisah di `CREDITS.md`; lengkapi sumbernya sebelum laporan final.

**Sudah ada (dari sumber CC0):** #1 `ui_click_1-2`, #2 `music/intro`, #4 `win`, #5 `lose`, #6 `animal_saved_1-3`, #7 `tree_to_palm`, #8 `water_spray` (loop), #9 `footstep_1-4`, #10 `fire_out_1-2`, #11 `music/sabotage_tension`, #12 `fire_loop`, #13 `fire_spread_1-3`, #14 `music/gameplay`, #15 `water_refill`, #16 `tank_empty`, #17 `timer_warning` (loop), #18 `tree_burning_1-2`, #21 `saboteur_ignite`, #22 `saboteur_plant_seed`, #23 `boss_appear`, #24 `stage_start`, #25 `stage_clear_star`, #26 `ui_hover`, #27 `pause_in`/`pause_out`.

**Dari tim (13-14/11/2026):** `voice/hey_antek_aseng`, `footstep_grass_1-3` (dari satu langkah, 2 varian digeser nadanya), `animal_burned` (potongan suara api hutan), serta file buatan/pilihan tim yang menggantikan pilihan awal: `animal_panic_burung_hantu`, `animal_panic_orangutan`, `animal_panic_bekantan`, `boss_appear`, `stage_start`, `stage_clear_star` (satu suara untuk 1-3 bintang), `tree_to_palm`, `saboteur_plant_seed`, `saboteur_ignite`, dan `music/gameplay`. Lisensi file-file ini belum diverifikasi, lihat `assets/audio/CREDITS.md`.

**Masih kosong:** langkah tanah terpisah (opsional; sementara `footstep_1-4` generik); suara satwa asli bila nanti ingin diganti.

**Catatan teknis:**
- Musik berformat **OGG** (bukan WAV) agar ukuran wajar: 4 lagu total ±7,5 MB, sedangkan WAV stereo bisa 15 MB atau lebih per lagu. Pemutarnya butuh decoder OGG; ini dependency kategori (b), tanya dosen dulu (lihat `05-implementation-plan.md`, Log Keputusan).
- Satu lagu `music/gameplay` dipakai untuk semua stage.
- `fire_spread_*` dan `ui_click_*` sangat pendek (0,03-0,1 dtk). Bila terasa kurang, cari yang lebih panjang.
- Total ukuran audio ±14 MB.
