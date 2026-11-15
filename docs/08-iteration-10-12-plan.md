# Rencana Iterasi 10–12 — Burn The Wo

Rencana ini memetakan sisa pekerjaan dari audit commit `8a37c64` ke tiga checkpoint pengumpulan. Fondasi engine dan level pemadaman sudah tersedia, termasuk intro, HUD, audio dasar, dan layar hasil. Istilah **Iterasi** mengikuti nomor pengumpulan, sedangkan **Stage** adalah level di dalam gim.

| Checkpoint | Cakupan | Bukti pengumpulan |
|---|---|---|
| **Iterasi 10 · Implementasi 4** | Satwa bisa diselamatkan, tiga stage tersambung, sabotase dan boss versi dasar, aturan hasil lengkap, serta uji build Windows. | Rekaman satu sesi sampai ending yang memperlihatkan rescue, sabotase, boss, dan hasil. Sertakan bukti jalur kalah dan hash build. |
| **Iterasi 11 · Implementasi 5** | Pause/resume, skor 1–3 bintang, UI hasil, SFX mekanik baru, perbaikan pencarian aset audio dan narasi, serta balancing awal. | Rekaman tiga stage dengan UI dan audio aktif, catatan uji pemain, dan perubahan parameter. |
| **Iterasi 12 · Implementasi 6** | Regresi dan perbaikan bug, QA paket Release x86, polish aset dan kontrol rilis, serta sinkronisasi laporan. | ZIP Windows yang diuji di laptop lain, petunjuk main, dan laporan yang merujuk ke versi build. |

## Iterasi 10 — Urutan implementasi

### 0. Buat branch implementasi dan cek build awal

Mulai dari `main` terbaru. Buat branch baru **sebelum mengubah atau memperbaiki kode**. Gunakan branch `feat/iteration-10-core-gameplay` untuk seluruh pekerjaan Iterasi 10, lalu buka PR ke `main` setelah verifikasi. Jalankan perintah ini pada checkout dengan working tree bersih.

```bash
git fetch origin
git switch -c feat/iteration-10-core-gameplay origin/main
```

- [ ] Jalankan build Debug dan Release x86 dengan MSVC v145. Uji alur intro → menu → level → hasil.
- [ ] Cek konstruktor `ResultScene::Summary` yang gagal pada GCC saat audit. Jika build target terdampak, perbaiki di branch ini sebelum menambah fitur. Simpan log build awal untuk membedakan masalah lama dari regresi.
- [ ] Pertahankan pembagian Engine dan Game. Pakai `EntityWorld`, collision grid, dan scene manager yang sudah tersedia untuk fitur baru.

### 1. Kunci aturan hasil dan konfigurasi tiga stage

- [ ] Sepakati cara rescue, batas waktu terpapar api, hasil pada tick yang sama dengan timeout, dan interaksi antara pohon hangus dan penanaman sawit. Catat keputusan tim di log [rencana implementasi](05-implementation-plan.md). Usulan awal untuk rescue adalah satwa kabur ke zona aman dan dihitung selamat saat tiba, sedangkan paparan api berkelanjutan menyebabkan status terbakar.
- [ ] Simpan konfigurasi stage pada satu tempat. Setiap stage membawa daftar satwa, target selamat, titik api awal, durasi, interval sebar, ambang sawit, dan jenis musuh. Teruskan konfigurasi ke `GameplayScene`, `StageRules`, dan sistem api yang memerlukan parameter per stage.
- [ ] Perluas `StageRules` agar menang mensyaratkan semua api padam dan kuota satwa tercapai. Tangani kalah karena ambang sawit, kuota mustahil tercapai, atau waktu habis. Ambil snapshot statistik ketika hasil ditetapkan dan hentikan update gameplay selama jeda layar hasil.

### 2. Selesaikan satu alur rescue, lalu aktifkan tiga spesies

- [ ] Mulai dari orang utan di Stage 1. Pakai sprite empat arah dan lifecycle `EntityWorld`. Implementasikan state diam → panik → kabur → selamat atau terbakar.
- [ ] Gunakan collision grid untuk mencari rute sederhana ke zona aman. Pilih titik spawn yang bisa dijangkau dari area bermain dan uji jalur yang terhalang pohon atau kolam.
- [ ] Hitung setiap satwa satu kali saat statusnya final. Tampilkan jumlah selamat dan target di HUD, lalu teruskan statistik ke layar hasil.
- [ ] Pakai implementasi yang sama untuk bekantan dan burung hantu. Susunan spesies mengikuti desain stage.

### 3. Sambungkan progression dan aktifkan antagonis

| Stage | Satwa | Hambatan minimum |
|---|---|---|
| **1. Api Pertama** | Orang utan | Pemadaman, isi ulang air, dan rescue. |
| **2. Sabotase** | Orang utan, bekantan | Anak buah menyalakan pohon lewat sistem api yang ada dan menanam bibit di lahan hangus. |
| **3. Hutan Terakhir** | Orang utan, bekantan, burung hantu | Wowo dan Tedduy menghadang pemain dengan satu pola gerak yang terlihat jelas. |

- [ ] Alirkan indeks stage dari menu ke gameplay dan layar hasil. Menang di Stage 1–2 membuka stage berikutnya. Menang di Stage 3 menutup rangkaian. Kalah menawarkan ulang stage yang sama dengan timer, api, air, satwa, dan musuh yang diinisialisasi kembali.
- [ ] Tambahkan state lahan hangus yang bisa ditanami. Pastikan perubahan pohon, lahan hangus, dan sawit konsisten dengan minimap, collision, serta persentase hutan dan sawit. Uji bahwa anak buah hanya menanam pada lahan yang valid.
- [ ] Hubungkan sprite Wowo dan Tedduy ke entitas Stage 3. Gunakan collision yang ada saat mereka bergerak dan menghadang. Sisakan variasi perilaku serta tuning lebih lanjut untuk hasil playtest.

### 4. Verifikasi sebelum membuka PR

| Uji | Hasil yang harus terlihat |
|---|---|
| Padamkan api sebelum target rescue tercapai | Stage masih berjalan selama target masih bisa dicapai dan waktu masih tersedia. |
| Rescue dan satwa terbakar | Penghitung HUD akurat. Kuota yang sudah mustahil dicapai memicu kalah. |
| Sabotase di Stage 2 dan boss di Stage 3 | Penyalaan api, penanaman pada lahan hangus, dan aksi menghadang dapat diamati dalam satu sesi. |
| Menang, kalah, retry, dan ending | Stage 1–2 bisa dilanjutkan, Stage 3 menuju ending, dan retry mengulang stage yang benar dengan state baru. |
| Ambang sawit, timeout, dan jeda hasil | Prioritas sesuai keputusan tim. Statistik tetap sama dari keputusan hasil sampai layar hasil. |

- [ ] Jalankan build Debug dan Release x86, mainkan alur dari menu sampai ending, dan cek tiap kondisi kalah. Catat hasil aktual, commit yang diuji, dan bukti demo di PR.

## Hasil implementasi Iterasi 10

- Mendekati satwa atau munculnya api di dekatnya memicu panik. Satwa lalu menuju zona aman di tengah peta dan dihitung selamat saat tiba. Paparan dekat api selama dua detik berturut-turut membuat satwa terbakar. Minimap menandai satwa yang masih bisa diselamatkan dan lokasi zona aman.
- Di Stage 2, pohon yang terbakar habis menjadi lahan hangus yang dapat ditanami anak buah. Pada Stage 1 dan 3, pohon yang terbakar habis langsung berubah menjadi sawit. Lahan hangus mengurangi persentase hutan, sedangkan ambang kalah menghitung tile sawit. Anak buah memakai sprite Wowo sementara.
- Wowo dan Tedduy bergerak mendekati penjaga hutan di Stage 3. Penjaga bergerak lebih lambat dalam jangkauan hadangan. Ketiga stage menggunakan peta dasar yang sama dengan konfigurasi api, satwa, lawan, dan durasi berbeda.
- Prioritas hasil adalah ambang sawit, target satwa yang sudah mustahil tercapai, timeout, lalu menang. Gameplay membeku ketika hasil diputuskan. Menang di Stage 3 menampilkan layar akhir **HUTAN SELAMAT**.
- Uji C++ mencakup jalur rescue mengitari tile padat, satwa terbakar, lahan hangus, penanaman dan penyalaan api, pengejaran boss, serta aturan hasil dan timeout. Verifikasi Windows MSVC v145, sesi manusia sampai ending, balancing, dan rekaman checkpoint masih terbuka.

## Iterasi 11 — UI, audio, dan balancing awal

- [ ] Tambahkan pause/resume yang menghentikan timer, AI, dan simulasi. Lengkapi layar hasil dengan penilaian 1–3 bintang dari persentase hutan dan jumlah satwa selamat, serta perjelas aksi lanjut, ulangi, dan kembali ke menu.
- [ ] Hubungkan SFX satwa, sabotase, boss, pause, dan bintang. Perbaiki path audio dan narasi untuk peluncuran dari folder kerja yang berbeda.
- [ ] Uji tiga stage dengan pemain lain. Setel waktu, titik api, laju sebar, kapasitas air, dan kuota satwa. Cek keterbacaan HUD, api, satwa, dan efek semprotan pada ukuran jendela berbeda.

## Iterasi 12 — Stabilitas dan rilis

- [ ] Ulangi uji menu hingga ending, jalur kalah dan retry, pause/resume, isi ulang, dan kondisi batas timer. Perbaiki temuan, rapikan placeholder yang masih mengganggu, dan keluarkan shortcut debug F1–F4 dari build rilis.
- [ ] Buat paket Windows Release x86 sesuai [panduan distribusi](07-build-dan-distribusi.md). Uji ZIP hasil ekstrak di laptop anggota lain dan cek aset, audio, DLL, serta petunjuk main.
- [ ] Koreksi checklist Tahap 3 yang berulang di laporan Iterasi 9. Lengkapi bukti implementasi, diagram engine, kredit aset dan audio, serta disclaimer penggunaan AI. Masukkan hasil playtest kelas setelah sesi berlangsung.
