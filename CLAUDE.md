# CLAUDE.md — Burn The Wo

Proyek tugas akhir mata kuliah **Pemrograman Gim** (Informatika, Universitas Brawijaya) oleh **Tim Dapur Tedduy**.
Gim: **Burn The Wo** — arcade 2D pixel art, satire/dark humor tentang kebakaran hutan.
(Nama folder/solution di repo ini: `Burn The Wok`; nama resmi di laporan: **Burn The Wo**.)

## ATURAN KERAS (baca dulu)

Target nilai tim adalah **99 (maksimal)**. Rubrik dosen:

| Cara pembuatan gim | Nilai maksimal |
|---|---|
| **Game engine buatan sendiri** | **99** ← TARGET KITA |
| Game engine pihak ke-3 (Unity, Godot, Unreal, dst.) | 89 |
| Game maker tanpa coding | 79 |

Konsekuensinya untuk kamu (Claude Code):

1. **JANGAN** menambahkan game engine atau game framework pihak ke-3 (Unity, Godot, Unreal, MonoGame, FNA, Stride, libGDX, Raylib, SFML, dll.). Sekali saja dipakai, plafon nilai turun ke 89.
2. Semua subsistem inti ditulis sendiri oleh tim: game loop, scene/state manager, renderer 2D (sprite/batch), input, kamera, collision, entity/AI, UI, audio mixer sederhana.
3. **Tanya dulu sebelum menambah dependency/NuGet/library apa pun.** Binding OpenGL dan windowing **sudah dikonfirmasi dosen tetap dihitung engine buatan sendiri**. Library level rendah lain (mis. decoder gambar/audio) tanya dulu. Lihat `docs/02-grading-constraints.md`.
4. Jangan menyalin/mem-fork engine open source ke repo.
5. Pisahkan kode **Engine** (generik, bisa dipakai gim lain) dari kode **Game** (khusus Burn The Wo). Ini bukti "engine sendiri" yang akan dilaporkan.

Rincian lengkap: [`docs/02-grading-constraints.md`](docs/02-grading-constraints.md).

## Peta dokumentasi

| File | Isi |
|---|---|
| `docs/README.md` | Indeks dokumen |
| `docs/01-project-overview.md` | Konsep, genre, platform, latar belakang, batasan satire |
| `docs/02-grading-constraints.md` | Syarat nilai maksimal & aturan engine |
| `docs/03-gameplay-design.md` | Mekanik, loop, stage, menang/kalah, kesulitan |
| `docs/04-characters-assets.md` | Karakter, item, gaya visual, spesifikasi aset |
| `docs/05-implementation-plan.md` | 7 tahap implementasi + jadwal + status |
| `docs/06-architecture.md` | Arsitektur engine & struktur proyek (isi/perbarui seiring kode) |
| `docs/07-build-dan-distribusi.md` | Membuat zip rilis (`tools/package-release.bat`) & mengirim ke dosen |

## Cara kerja

- Bahasa komunikasi dengan tim: **Indonesia**. Komentar kode & nama identifier: Inggris.
- Kerjakan **per tahap** sesuai `docs/05-implementation-plan.md`. Jangan loncat ke fitur tahap berikutnya.
- Setelah menyelesaikan sesuatu, perbarui status di `docs/05-implementation-plan.md`.
- Jika desain di laporan ambigu, **tanya**, jangan mengarang. Catat keputusan di `docs/05-implementation-plan.md` bagian "Log Keputusan".
- Penggunaan AI wajib transparan: laporan memuat *Disclaimer Penggunaan AI*. Tim harus paham dan bisa menjelaskan semua kode yang dihasilkan.
- Prioritas: **gim yang bisa dimainkan di setiap akhir tahap** > fitur lengkap tapi belum jalan.

## Tech stack

Lihat bagian "Tech Stack" di `docs/06-architecture.md` (isi sesuai kondisi repo; jangan menebak).
