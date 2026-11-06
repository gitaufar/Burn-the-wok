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
