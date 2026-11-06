# 03 — Gameplay Design

## Ringkasan

Burn The Wo adalah game 2D arcade berbasis waktu (time-based survival). Pemain berperan sebagai **penjaga hutan** (karakter yang dikendalikan pemain sendiri) yang harus memadamkan kebakaran, menyelamatkan satwa, dan mencegah hutan berubah menjadi perkebunan kelapa sawit akibat sabotase **Wowo** dan **Tedduy**.

> Penjaga hutan **adalah pemain**, terpisah dari Wowo dan Tedduy. Wowo dan Tedduy adalah antagonis/sabotase yang dikendalikan AI. Sprite penjaga hutan masih perlu dibuat (lihat `04-characters-assets.md`).

## Mekanisme utama

1. **Memadamkan api** — pemain menyemprot air ke titik api sebelum menyebar ke seluruh hutan.
2. **Mengelola air** — persediaan air terbatas; pemain harus mengisi ulang di sumber air secara berkala.
3. **Menyelamatkan satwa** — orang utan, bekantan, dan burung hantu harus dilindungi dari kebakaran.
4. **Mencegah perubahan lahan** — pohon terbakar yang tidak dipadamkan dalam waktu tertentu berubah menjadi **pohon sawit**, sehingga pemain harus memprioritaskan area yang terancam.
5. **Menghadapi sabotase** — anak buah Wowo dan Tedduy menyebarkan api, menanam bibit sawit, dan (tahap akhir) Wowo & Tedduy turun langsung menghalangi pemain.

## Gameplay loop

```
Mendeteksi api → Memadamkan api → Mengisi ulang air → Menyelamatkan satwa → Menghadapi sabotase
        ↑___________________________ ulangi hingga waktu habis ___________________________|
```

## Kondisi menang & kalah

- **Menang:** seluruh api berhasil dipadamkan **dan** target satwa berhasil diselamatkan sebelum waktu habis.
- **Kalah:** perkebunan sawit mencapai batas yang ditentukan, **atau** target penyelamatan satwa gagal terpenuhi.

## Progression (3 stage)

| Stage | Kondisi | Hambatan | Satwa |
|---|---|---|---|
| 1. Api Pertama | Titik api sedikit dan menyebar pelan. Pemain belajar menyemprot dan mengisi ulang air. | Belum ada hambatan. | Orang utan |
| 2. Sabotase | Titik api bertambah dan menyebar lebih cepat. | Anak buah menyalakan api baru dan menanam bibit sawit di lahan hangus. | Orang utan, bekantan |
| 3. Hutan Terakhir | Api menyebar sangat cepat dengan waktu paling singkat. | Wowo dan Tedduy turun langsung menghadang pemain. | Orang utan, bekantan, burung hantu |

### Objective tiap stage
- Memadamkan api sebelum menyebar ke seluruh area.
- Menyelamatkan satwa yang terjebak (satwa terkena api bila terlalu lama).
- Menjaga pohon agar tidak berubah menjadi pohon sawit.

### Peningkatan kesulitan
- Api menyebar makin cepat di tiap stage.
- Hambatan dari Wowo dan Tedduy makin banyak.
- Waktu makin singkat dan jumlah satwa yang harus diselamatkan bertambah.

### Penilaian akhir
Skala **1–3 bintang** berdasarkan **persentase hutan tersisa** dan **jumlah satwa yang diselamatkan**.

## Kontrol (usulan, bisa diubah)

| Aksi | Input |
|---|---|
| Gerak | WASD / panah |
| Arah semprot | Posisi mouse |
| Semprot air | Klik kiri (tahan) |
| Isi ulang air | Berdiri di sumber air / tombol interaksi |
| Pause | Esc |

## Parameter balancing (nilai awal, perlu diuji di Tahap 7)

Semua angka di bawah **placeholder**; simpan di satu file konfigurasi agar mudah diubah.

| Parameter | Stage 1 | Stage 2 | Stage 3 |
|---|---|---|---|
| Durasi stage (detik) | _TBD_ | _TBD_ | _TBD_ |
| Jumlah titik api awal | _TBD_ | _TBD_ | _TBD_ |
| Kecepatan penyebaran api | _TBD_ | _TBD_ | _TBD_ |
| Waktu pohon terbakar → sawit | _TBD_ | _TBD_ | _TBD_ |
| Kapasitas air maksimum | _TBD_ | _TBD_ | _TBD_ |
| Target satwa selamat | _TBD_ | _TBD_ | _TBD_ |
| Batas sawit untuk kalah (%) | _TBD_ | _TBD_ | _TBD_ |
