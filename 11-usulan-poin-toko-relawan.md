# Usulan: Poin, Toko, Relawan, dan Rescue dengan Spasi

> **Status: DRAF USULAN, belum disepakati tim.** Semua angka adalah tebakan awal dan perlu disetel lewat playtest. Jika disetujui, catat keputusannya di Log Keputusan [`05-implementation-plan.md`](05-implementation-plan.md).

## 0. Kondisi kode sekarang (titik berangkat)

| Hal | Kondisi di `main` (2f85dbd) | Dampak ke usulan |
|---|---|---|
| Simpan progres | **Belum ada.** Tidak ada file save; progres stage hilang saat gim ditutup | Poin dan pembelian butuh **sistem save** lebih dulu |
| Menu | Hanya tombol **MULAI**, selalu ke Stage 1 | Perlu tombol **TOKO** dan (sebaiknya) **pilih stage / lanjutkan** |
| Rescue | Satwa panik jika ranger mendekat **atau** api < 30 px, lalu kabur sendiri ke tengah peta (`StageActor::StepAnimal`) | Satwa sering selamat tanpa aksi pemain, jadi perlu diubah (bagian 4) |
| Layar hasil | `ResultScene::Summary` sudah membawa hutan %, satwa, waktu sisa, dan bintang | Tinggal ditambah baris poin |
| Tombol Spasi | Saat gameplay hanya dipakai di panel pause | Bebas dipakai untuk rescue |
| Font piksel | Huruf besar, angka, dan beberapa tanda baca | Mungkin perlu glyph `+` (untuk "+25") |

**Jadwal:** hari ini 23/11 (Tahap 6, UI/audio/polish) dan Tahap 7 (finalisasi) pada 30/11. Fitur ini menambah lingkup cukup besar, jadi di bagian 7 dibagi menjadi **MVP** dan **opsional**.

---

## 1. Sistem poin

### Cara kerja
- Poin didapat **saat stage berakhir**, dihitung dari ringkasan stage, lalu ditambahkan ke **tabungan** yang disimpan di file save.
- Saat satwa diselamatkan, muncul teks melayang **"+25"** di atas satwa, tetapi poinnya baru masuk tabungan di akhir stage. Ini mencegah eksploit keluar di tengah permainan.
- Layar hasil menampilkan rincian poin dengan animasi menghitung naik, seperti animasi bintang yang sudah ada.

### Rumus usulan

| Sumber | Poin | Alasan |
|---|---|---|
| Menang Stage 1 / 2 / 3 | **100 / 150 / 200** | Stage makin sulit |
| Tiap satwa selamat | **+25** (orang utan, bekantan) / **+35** (burung hantu, hanya di Stage 3) | Permintaan utama: rescue memberi poin |
| Tiap bintang | **+25** per bintang | Mendorong main lebih rapi |
| Sisa waktu | **+1 per 2 detik** tersisa | Bonus kecil untuk kecepatan |
| Semua satwa selamat | **+50** bonus "Penyelamat" | Tujuan ekstra |
| **Kalah** | Hanya **50%** dari poin satwa yang selamat, tanpa poin menang | Tetap ada progres walau kalah, tapi tidak bisa farming |
| **Mengulang stage yang sudah dimenangkan** | Poin menang **dikali 0,5** | Mencegah farming Stage 1 |

### Contoh perhitungan
- **Stage 1, main biasa** (menang, 1 dari 2 satwa, 1 bintang, sisa 40 detik): 100 + 25 + 25 + 20 = **170**
- **Stage 1, sempurna** (2 satwa, 3 bintang, sisa 70 detik): 100 + 50 + 75 + 35 + 50 = **310**
- **Stage 3, sempurna** (4 satwa: 2 burung hantu, 3 bintang, sisa 50 detik): 200 + 25 + 25 + 70 + 75 + 25 + 50 = **470**
- Total kasar satu kali main sampai tamat: **±500–1.000 poin**. Harga toko di bawah disesuaikan dengan angka ini.

### Implementasi
- `game/systems/PointCalculator.h`: fungsi murni seperti `StarRating.h` (input `Summary`, output rincian poin). Mudah diuji.
- `ResultScene::Summary` ditambah `PointBreakdown`, lalu `ResultScene` menambahkan poin ke `SaveData` dan menyimpan **sekali** saat masuk scene.

---

## 2. Sistem save (wajib sebelum poin dan toko)

### Cara kerja
- File teks sederhana `save.txt` (format `kunci=nilai`) di samping `.exe`. Jika folder tidak bisa ditulis, simpan ke `%APPDATA%\BurnTheWo\`.
- Isinya: `points`, `stage_unlocked`, `stage1_best_stars`, `stage1_cleared`, ..., `upgrade_tank=2`, `volunteers=1`, dan seterusnya.
- File rusak atau tidak ada: mulai dari nol, tanpa crash.

### Pembagian Engine / Game (penting untuk nilai 99)
- **Engine:** `engine/core/KeyValueFile` (baca/tulis `kunci=nilai`, generik, memakai `std::fstream`). Tanpa library baru.
- **Game:** `game/progress/SaveData` (arti tiap kunci: poin, upgrade, stage).

### Tambahan menu
- Tombol **LANJUTKAN** (ke stage tertinggi yang terbuka) dan **PILIH STAGE** (Stage 1–3 dengan bintang terbaik). Tanpa ini, poin yang sudah ditabung tidak berguna karena menu selalu mengulang Stage 1.
- Tombol **RESET PROGRES** di toko atau menu (dengan konfirmasi), berguna untuk demo ke dosen.

---

## 3. Toko (Store)

### Kapan muncul
- Tombol **TOKO** di menu muncul (atau tidak lagi terkunci dengan ikon gembok) setelah **Stage 1 pertama kali dimenangkan**. Saat pertama muncul, beri penanda **"BARU!"** yang berkedip.

### Layout
- Scene baru `StoreScene`: daftar barang di kiri (ikon, nama, level, harga), panel deskripsi di kanan, saldo poin di kanan atas. Navigasi dengan mouse, panah, dan Enter (sama seperti panel pause). Esc untuk kembali.
- Barang yang poinnya tidak cukup ditampilkan abu-abu dan diberi SFX gagal.

### Daftar barang

| # | Barang | Efek | Level & harga | Kode yang disentuh |
|---|---|---|---|---|
| 1 | **Tangki Besar** | Kapasitas air 100 → 125 → 150 → 180 | Lv1 120, Lv2 220, Lv3 350 | `TankCapacity` jadi nilai runtime dari save |
| 2 | **Nozel Turbo** | Daya semprot (`DropPower`) +15% per level | Lv1 150, Lv2 280 | `Sprayer` |
| 3 | **Pompa Cepat** | Isi ulang di kolam +40% | Lv1 100, Lv2 200 | `RefillPerSecond` |
| 4 | **Sepatu Lapangan** | Kecepatan jalan +10% per level | Lv1 120, Lv2 240 | `PlayerSpeed` |
| 5 | **Relawan Tetap** | +1 relawan AI di setiap stage (maks 2) | Relawan 1 400, Relawan 2 700 | Entity baru (bagian 5) |
| 6 | **Iklankan Kampanye** | Selama 10 menit waktu bermain, tiap stage bisa memanggil 2–3 relawan selama 30–45 detik (bagian 6) | Lokal 150, Nasional 300 | Sama dengan #5 + timer kampanye di save |

**Prinsip balancing:** dengan 170 poin dari Stage 1 biasa, pemain bisa langsung membeli satu upgrade murah. Relawan tetap baru terjangkau setelah 2–3 stage atau setelah mengulang dengan rapi. Upgrade **tidak boleh** membuat Stage 3 terlalu mudah, jadi total semua upgrade sebaiknya tidak menggandakan kekuatan pemain.

### Implementasi
- `game/progress/Upgrades.h`: tabel barang (id, nama, harga per level, efek) di satu tempat seperti `GameConfig.h`.
- Saat `GameplayScene` dibuat, ia membaca level upgrade dari `SaveData` lalu menerapkannya ke `Player`, `Sprayer`, dan tangki. Konstanta di `GameConfig.h` menjadi **nilai dasar**.

---

## 4. Rescue satwa dengan tombol Spasi

### Masalah sekarang
Satwa panik dan kabur sendiri begitu api mendekat, jadi rescue sering terjadi tanpa sengaja. Jika rescue cukup dengan menekan Spasi tetapi satwa tetap bisa kabur sendiri, tombolnya jadi tidak berarti.

### Usulan (direkomendasikan): "Tenangkan lalu tuntun"
1. **Terjebak:** api di dekatnya membuat satwa **meringkuk di tempat** (gemetar, ikon **"!"** merah di atas kepala). Satwa **tidak** kabur sendiri. Hitungan paparan api tetap berjalan, jadi satwa bisa terbakar jika dibiarkan.
2. **Dalam jangkauan:** saat ranger berada dalam ±28 px, muncul petunjuk **[SPASI]** di atas satwa.
3. **Tahan Spasi 0,8 detik:** muncul cincin progres dan ranger berhenti bergerak (masih bisa menyemprot). Ini memberi pilihan: padamkan api dulu atau selamatkan dulu.
4. **Selesai:** SFX panik satwa, teks "+25", lalu satwa **kabur ke zona aman** memakai rute yang sudah ada (`FindRoute`). Setelah sampai, statusnya Selamat.
5. Satwa yang **tidak** terancam api tetap diam di tempat. Pemain juga boleh menyelamatkannya lebih awal dengan Spasi.

Perubahan kode: state `Rescue` ditambah `Trapped` dan `Calming`. `StepAnimal` tidak lagi otomatis Panicked/Fleeing. `GameplayScene` mengirim status "Spasi ditahan" ke satwa terdekat. Aturan menang/kalah di `StageRules` tidak perlu diubah.

### Alternatif

| Opsi | Cara | Kelebihan | Kekurangan |
|---|---|---|---|
| A. Tekan Spasi sekali | Satu tekan, satwa langsung kabur | Paling cepat dibuat | Kurang menegangkan |
| **B. Tahan 0,8 detik (rekomendasi)** | Seperti di atas | Ada risiko dan keputusan, mudah dibuat | Perlu cincin progres |
| C. Gendong | Spasi mengangkat satwa, ranger lambat 30% dan tidak bisa menyemprot, Spasi lagi di zona aman untuk melepas | Paling seru, satwa terasa "diselamatkan" | Butuh sprite menggendong dan zona aman yang jelas, paling lama dibuat |

**Usul tambahan:** tandai **zona aman** di peta dengan ikon/bendera (sekarang tengah peta tidak ditandai) dan di minimap.

---

## 5. Relawan (tetap dan kampanye)

### Cara kerja AI relawan
- Entity baru `game/entities/Volunteer` (memakai `engine::Entity` dan `EntityWorld`).
- Siklus: cari **api terdekat** → jalan ke sana memakai `FindRoute` milik `StageActor` (dipindah ke fungsi bersama) → semprot dengan tangki kecil (40 unit, daya 60% dari ranger) → jika air habis, jalan ke kolam terdekat dan isi ulang → ulangi.
- Relawan **tidak** menyelamatkan satwa, supaya rescue tetap tugas pemain dan poin satwa tetap bermakna.
- Di Stage 3, boss Wowo/Tedduy bisa menghadang relawan juga (opsional).
- Relawan **tetap** muncul di setiap stage sesuai jumlah yang dibeli dan bertugas sepanjang stage. Relawan **kampanye** hanya datang saat kampanye aktif dan bertugas 30–45 detik (bagian 6).

---

## 6. "Iklankan Kampanye" (relawan sementara dari toko)

Bukan iklan sungguhan: tidak ada SDK iklan atau jaringan, karena itu melanggar aturan dependency dan gimnya offline. "Iklan" di sini adalah **barang di toko**: pemain membayar poin untuk mengiklankan kampanye pemadaman, dan kampanye yang sedang aktif mendatangkan relawan tambahan.

### Cara kerja
1. Di toko pemain membeli **"IKLANKAN KAMPANYE"**. Muncul animasi singkat 2–3 detik berupa poster kampanye fiktif yang "viral", misalnya *"#JagaHutanKita"*. Animasi ini bisa dilewati.
2. Kampanye **aktif selama 10 menit waktu bermain**.
3. Selama kampanye aktif, **setiap kali masuk stage**, sekelompok relawan kampanye datang membantu selama **30 detik**, lalu pergi.
4. Setelah 10 menit habis, kampanye berakhir dan bisa dibeli lagi.

### Keputusan desain yang perlu dipilih

**a. "10 menit" dihitung bagaimana?**

| Opsi | Cara hitung | Kelebihan | Kekurangan |
|---|---|---|---|
| **Waktu bermain (rekomendasi)** | Hanya berkurang saat berada di dalam stage dan tidak pause. Sisa waktunya disimpan di save | Adil: tidak hangus saat di menu, toko, atau gim ditutup | Pemain harus diberi tahu sisa waktunya |
| Waktu nyata (jam komputer) | Berkurang terus, termasuk saat gim ditutup | Terasa seperti "kampanye sungguhan" | Bisa hangus tanpa dipakai; bisa dicurangi dengan mengubah jam PC |

Satu stage berlangsung 115–150 detik, jadi 10 menit waktu bermain kira-kira **4–5 stage**.

**b. Kapan relawan kampanye datang di dalam stage?**

| Opsi | Cara | Catatan |
|---|---|---|
| **Dipanggil pemain (rekomendasi)** | Tekan **Q** ("PANGGIL RELAWAN"), sekali per stage. Relawan masuk dari tepi peta dan bertugas 30 detik | Ada strategi: simpan untuk saat api menyebar paling parah |
| Otomatis di awal stage | Relawan datang saat stage dimulai | Paling sederhana; di awal api masih sedikit, jadi kurang terasa |
| Otomatis saat darurat | Datang saat api ≥ X titik atau sawit mendekati batas | Terasa dramatis, tapi pemain tidak memegang kendali |

### Tingkat kampanye (usulan angka)

| Barang | Harga | Durasi kampanye | Relawan per stage | Lama bertugas |
|---|---|---|---|---|
| **Kampanye Lokal** | 150 | 10 menit | 2 | 30 detik |
| **Kampanye Nasional** (terbuka setelah Stage 2 dimenangkan) | 300 | 10 menit | 3 | 45 detik |

- Membeli lagi saat kampanye masih aktif **menambah durasi** (maksimal 20 menit), tidak menambah jumlah relawan.
- Relawan kampanye memakai AI yang sama dengan Relawan Tetap (bagian 5). Perbedaannya hanya timer tugas: setelah 30 detik mereka berjalan ke tepi peta lalu hilang.

### Tampilan
- **Menu dan toko:** ikon megafon dan sisa durasi, misalnya "KAMPANYE AKTIF 7:32".
- **HUD gameplay:** ikon megafon dan petunjuk **[Q] PANGGIL**. Saat relawan bertugas, tampilkan bar 30 detik yang menyusut.
- **Layar hasil:** sisa durasi kampanye setelah stage.

### Implementasi
- `SaveData` menyimpan `campaign_tier` dan `campaign_seconds_left`.
- `GameplayScene` mengurangi `campaign_seconds_left` setiap update saat tidak pause, lalu menyimpannya di akhir stage.
- Tombol Q men-spawn N `Volunteer` dengan `dutySeconds = 30`.

### Catatan satire
Poster dan nama kampanye harus fiktif. Jangan meniru merek, logo, tagar organisasi, atau orang sungguhan. Periksa batasan satire di `01-project-overview.md`. Ide satire opsional: tingkat tertinggi "disponsori" perusahaan sawit fiktif yang ikut membakar hutan.

---

## 7. Prioritas dan perkiraan kerja

| Prioritas | Fitur | Perkiraan |
|---|---|---|
| **MVP 1** | Save file (Engine `KeyValueFile` + Game `SaveData`) | 0,5 hari |
| **MVP 1** | Poin + rincian di layar hasil | 0,5 hari |
| **MVP 1** | Rescue Spasi (opsi B) + penanda zona aman | 1 hari |
| **MVP 2** | Menu: LANJUTKAN / PILIH STAGE / TOKO (terkunci sampai Stage 1 menang) | 0,5 hari |
| **MVP 2** | Toko + 4 upgrade statistik (#1–4) | 1 hari |
| Opsional | Relawan AI (dipakai Relawan Tetap dan Kampanye) | 1,5–2 hari |
| Opsional | Iklankan Kampanye (timer di save, tombol Q, HUD) setelah Relawan AI jadi | 0,5–1 hari |
| Opsional | Rescue opsi C (gendong) | +1 hari |

Dengan sisa ±7 hari sampai 30/11, **MVP 1 + MVP 2 realistis**. Relawan dan iklan sebaiknya dikerjakan hanya jika MVP sudah stabil, karena Tahap 7 (playtest dan rilis) tetap harus jalan.

---

## 8. Aset yang dibutuhkan

### Grafis
| Aset | Ukuran usulan | Untuk | Prioritas |
|---|---|---|---|
| Ikon poin/koin (misal daun emas) | 16×16 | HUD, hasil, toko | MVP |
| Ikon barang toko: tangki, nozel, pompa, sepatu, relawan, megafon kampanye | 32×32 masing-masing | Toko | MVP |
| Ikon gembok | 16×16 | Tombol TOKO dan stage terkunci | MVP |
| Panel/latar toko (bisa digambar dengan `DrawRect` seperti menu) | Tidak wajib | Toko | Opsional |
| Bubble **"!"** dan petunjuk **[SPASI]** (bisa digambar kode, gaya tombol `key()` di menu) | 12×12 | Rescue | MVP |
| Cincin progres (bisa digambar kode) | Tidak wajib | Rescue | MVP |
| Bendera/tenda **zona aman** | 32×32 | Peta + minimap | MVP |
| Satwa **meringkuk/gemetar**, 2 frame per spesies (atau cukup efek getar di kode) | Sesuai sprite satwa | Rescue | Opsional |
| **Sprite relawan** 4 arah × 4 frame (recolor ranger: rompi warna lain, tanpa kacamata) | Sesuai ranger | Relawan | Opsional |
| Ranger menggendong satwa (hanya opsi C) | Sesuai ranger | Rescue C | Opsional |
| 1–2 **poster kampanye** fiktif (Lokal, Nasional) | 1280×720 seperti `intro/scene_*.png`, atau lebih kecil untuk pop-up | Animasi beli kampanye | Opsional |
| Ikon megafon kecil | 16×16 | HUD, menu, petunjuk [Q] | Opsional |
| Variasi warna rompi relawan kampanye (beda dari Relawan Tetap) | Recolor | Relawan kampanye | Opsional |
| Glyph `+` (dan `:` jika belum ada) di `PixelText` | Kode | "+25", rincian poin | MVP |

### Suara (WAV, sesuai format mixer yang ada)
| SFX | Untuk |
|---|---|
| `points_gain` (tik-tik naik) | Animasi hitung poin di layar hasil |
| `purchase_ok`, `purchase_fail` | Toko |
| `upgrade_level` | Toko |
| `rescue_calm` (napas/suara lembut saat menahan Spasi) | Rescue |
| `store_unlock` | Pertama kali toko terbuka |
| `volunteer_join` | Relawan muncul di awal stage |
| `campaign_launch` (jingle/sorakan pendek, ±2 detik) | Membeli kampanye |
| `volunteer_call` (peluit/megafon) | Menekan Q memanggil relawan kampanye |
| `volunteer_leave` | Relawan kampanye selesai bertugas |
| Musik toko (opsional, bisa memakai `menu.wav`) | Toko |

Jangan lupa mencatat sumber/lisensi aset baru di `assets/audio/CREDITS.md`.

---

## 9. Pertanyaan untuk tim sebelum mulai

1. Rescue: opsi **B (tahan Spasi)** atau **C (gendong)**?
2. Apakah satwa yang terancam api **masih boleh kabur sendiri**, atau harus ditolong (usulan: harus ditolong)?
3. Kalah: dapat sebagian poin (usulan 50% poin satwa) atau nol?
4. Relawan dan kampanye masuk lingkup sebelum 30/11, atau dijadikan opsional/pasca-pengumpulan?
5. Kampanye: "10 menit" dihitung dari **waktu bermain** (usulan) atau waktu nyata? Relawan kampanye **dipanggil dengan Q** (usulan) atau datang otomatis?
6. Siapa yang mengerjakan aset ikon dan sprite relawan?
