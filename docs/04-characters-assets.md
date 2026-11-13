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
| `items/sprayer_left/right.png` | **Bahan** sprite gabungan penjaga hutan (lihat di bawah); tidak lagi digambar terpisah oleh gim | — |
| `characters/ranger/<arah>_<n>.png` | Penjaga hutan **berjalan** dengan tangki di punggung (16 frame, 96×80 px) — sprite pemain di gim | Dibuat oleh `tools/assets/compose_ranger.py` |
| `characters/ranger/spray_<arah>_<n>.png` | Penjaga hutan **menyemprot**: memegang pistol semprot (16 frame) — dipakai selama klik kiri ditahan | idem |
| `items/water_spray_left/right.png` | Semburan air di moncong pistol semprot | Hanya saat air keluar dan pemain menghadap kiri/kanan (gambar hanya menghadap kiri/kanan); tetes air (`WaterDrop`, kotak 2 px) tetap ada karena tetes itulah yang mengenai api |

Semua gambar item dipotong otomatis ke bagian yang tidak transparan (`Image::GetContentBounds`). Belum dipakai: sprite satwa (Tahap 4) dan Wowo/Tedduy (Tahap 5).

#### Sprite gabungan penjaga hutan + penyemprot (13/11/2026)

Menggambar penyemprot sebagai gambar terpisah di atas badan terlihat tidak menyatu (melayang, menutupi badan, air tidak keluar dari moncong). Solusinya: 32 frame baru di `characters/ranger/` yang **digabung dari aset tim sendiri** (frame penjaga hutan `Front/Back/Left/Right *.png` + `items/sprayer_*.png`) oleh skrip `tools/assets/compose_ranger.py`:

- Penyemprot dipecah jadi tangki dan pistol, dikecilkan (0,36× dan 0,42×) agar proporsional dengan karakter.
- **Set jalan** (`<arah>_<n>.png`): hanya tangki di punggung, tangan mengayun bebas. Samping: tangki di belakang punggung (di bawah lapisan badan). Belakang: tangki menempel di punggung. Depan: tangki tersembunyi di belakang badan.
- **Set semprot** (`spray_<arah>_<n>.png`): kaki diambil dari frame jalan, badan atas dari frame berdiri, sehingga tangan diam memegang pistol sementara kaki tetap melangkah. Samping: pistol dipegang dua tangan (tangan digambar skrip). Depan/belakang: pistol lurus dipegang di samping badan mengarah ke bawah/atas (digambar skrip dengan warna yang sama, karena gambar pistol hanya ada tampak samping) — air keluar dari samping kaki, bukan dari tengah badan.
- Gim memakai set jalan, dan beralih ke set semprot selama klik kiri ditahan (`Player::Aim`).
- Semua frame diseragamkan: kanvas 96×80, kepala di tengah (x = 48), kaki di baris y = 76. Karena itu gim memakai satu jangkar tetap untuk semua frame, dan goyangan kecil akibat piksel nyasar di file asli hilang.
- Skrip mencetak posisi moncong per arah; angka itu dipakai `Player::GetNozzlePosition` sehingga air keluar tepat dari moncong.

Skrip ini alat bantu pengembangan (Python + Pillow), **bukan** dependency gim: gim hanya memuat PNG hasilnya. Jalankan ulang jika sprite karakter/penyemprot diganti: `python tools/assets/compose_ranger.py` (opsi `--preview file.png` untuk lembar pratinjau). File asli `Front 1.png` dst. tetap disimpan sebagai sumber.

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
