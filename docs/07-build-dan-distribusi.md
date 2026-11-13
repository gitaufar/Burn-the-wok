# 07 — Build & Distribusi (kirim gim ke dosen)

Panduan membuat file **zip** berisi gim yang bisa didownload dan dimainkan orang lain, lalu mengirimnya lewat **Google Drive** atau **itch.io**.

> Ringkas: jalankan `tools\package-release.bat` → tes zip di folder lain → upload zip → bagikan link.

## 0. Gambaran

- Gim ini program **Windows** (`.exe`), bukan aplikasi Android (APK) dan bukan website. Tidak bisa di-deploy ke Vercel/Netlify.
- Yang dibagikan: satu file `Burn-The-Wo_<tanggal>_<commit>.zip` (sekitar 0,6 MB untuk demo Tahap 1).
- Pemain cukup: download → ekstrak → klik dua kali `Burn The Wo.exe`. **Tidak perlu install apa pun**: runtime C++ sudah tertanam di `.exe` (build Release memakai `/MT`), dan `SDL2.dll` + `glew32.dll` ikut di dalam zip.
- Kebutuhan laptop pemain: Windows 10/11 (32/64-bit), kartu grafis dengan OpenGL 3.3 (hampir semua laptop sejak ~2012).

## 1. Membuat zip

Kebutuhan di laptop yang membuat zip: Visual Studio 2026 dengan workload **Desktop development with C++** (sama seperti untuk build biasa).

**Cara A — paling mudah:** buka folder `tools\` di File Explorer, klik dua kali **`package-release.bat`**. Tunggu sampai muncul tulisan hijau `SELESAI`, lalu tekan tombol apa saja untuk menutup.

**Cara B — dari terminal** (PowerShell atau terminal VS Code, di root repo):

```powershell
.\run package
# atau langsung:
powershell -ExecutionPolicy Bypass -File tools\package-release.ps1
```

Opsi: tambahkan `-SkipBuild` untuk membungkus hasil build Release yang sudah ada tanpa build ulang.

Yang dilakukan skrip:

1. Build `Burn The Wok.slnx` sebagai **Release | x86** (MSBuild dicari otomatis lewat `vswhere`).
2. Menyusun folder `dist\Burn-The-Wo\`.
3. Membuat `dist\Burn-The-Wo_<yyyy-MM-dd>_<hash-commit>.zip`.

Isi zip:

```
Burn-The-Wo/
├── Burn The Wo.exe        # gim (nama resmi; di proyek bernama "Burn The Wok.exe")
├── SDL2.dll               # wajib satu folder dengan .exe
├── glew32.dll             # wajib satu folder dengan .exe
├── CARA MAIN.txt          # petunjuk untuk pemain (sumber: tools\package\CARA MAIN.txt)
├── licenses/              # lisensi SDL2 & GLEW (wajib ikut disebar bersama binary)
└── assets/                # otomatis ikut jika folder "Burn The Wok\assets" sudah ada
```

Catatan:
- Folder `dist\` tidak di-commit ke git (ada di `.gitignore`). Zip disebar lewat Drive/itch.io, bukan lewat repo.
- **Commit dulu sebelum membuat zip** yang akan dikirim, supaya hash di nama file menunjuk ke kode yang tepat. Hash diambil dari commit terakhir, jadi perubahan yang belum di-commit tidak tercermin di nama file.
- Untuk mengubah petunjuk pemain (mis. kontrol baru), edit `tools\package\CARA MAIN.txt`.

## 2. Jika skrip gagal

| Pesan | Penyebab & solusi |
|---|---|
| `Visual Studio tidak ditemukan` / `MSBuild tidak ditemukan` | Install VS 2026 + workload "Desktop development with C++" |
| `Build gagal` | Ada error kompilasi. Buka `Burn The Wok.slnx`, build `Release \| x86` di VS untuk melihat error-nya |
| `... cannot be loaded because running scripts is disabled` | Jangan klik kanan → "Run with PowerShell"; pakai `package-release.bat` atau perintah Cara B (sudah memakai `-ExecutionPolicy Bypass`) |
| `Compress-Archive` error karena file sedang dipakai | Tutup gim yang sedang berjalan dari folder `dist\`, jalankan ulang skrip |

## 3. Tes zip sebelum dikirim (wajib)

1. Copy zip ke folder lain (mis. Desktop), **klik kanan → Extract All...**
2. Buka folder hasil ekstrak, klik dua kali `Burn The Wo.exe`.
3. Coba: Enter (mulai), WASD (gerak), roda mouse (zoom), Esc (menu), Esc lagi (keluar).
4. Idealnya ulangi di **laptop anggota tim lain** yang tidak punya Visual Studio. Ini paling mendekati kondisi laptop dosen.

Jika muncul layar biru **"Windows protected your PC"**: klik **More info → Run anyway**. Ini normal untuk `.exe` tanpa sertifikat digital berbayar. Sebutkan juga di pesan ke dosen (template di bawah).

## 4. Kirim lewat Google Drive

1. Buka [drive.google.com](https://drive.google.com) dengan akun Google tim (disarankan satu akun/folder bersama, bukan akun pribadi acak).
2. (Sekali saja) Buat folder, mis. **`Burn The Wo - Build`**: klik **+ New → New folder**.
3. Masuk ke folder itu, klik **+ New → File upload**, pilih zip dari folder `dist\`.
   - **Jangan** upload folder `Burn-The-Wo` yang belum di-zip, dan jangan upload `.exe` saja: DLL-nya akan tertinggal.
4. Setelah upload selesai, klik kanan file zip → **Share → Share**.
5. Atur akses, pilih salah satu:
   - **Khusus dosen (disarankan):** ketik email dosen di kolom "Add people", pilih peran **Viewer**, centang **Notify people**, klik **Send**.
   - **Siapa saja yang punya link:** di bagian **General access**, ubah **Restricted** menjadi **Anyone with the link**, peran **Viewer**, lalu **Copy link**.
6. Kirim link ke dosen (template di bawah).
7. (Opsional) Cek sendiri: buka link di jendela **Incognito**. Jika muncul tombol Download, link sudah benar.

Catatan Drive:
- Drive tidak menampilkan isi zip, hanya tombol **Download**. Ini normal.
- Untuk versi baru, upload zip baru dengan nama baru (tanggal & hash berbeda). Jangan menimpa zip lama, supaya versi yang sudah dikirim tetap bisa dicek. Bisa juga klik kanan → **File information → Manage versions** jika ingin link tetap sama.

### Template pesan ke dosen

> Selamat siang Pak, kami dari tim Dapur Tedduy (Burn The Wo) ingin mengirimkan build gim kami:
> [link Drive / itch.io]
>
> Cara menjalankan:
> 1. Download file zip, lalu **ekstrak** (klik kanan → Extract All).
> 2. Buka folder hasil ekstrak, klik dua kali **Burn The Wo.exe**.
> 3. Jika muncul peringatan "Windows protected your PC", klik **More info → Run anyway** (muncul karena gim belum memiliki sertifikat digital).
>
> Kontrol: WASD untuk bergerak, roda mouse untuk zoom, Esc untuk kembali ke menu. Petunjuk lengkap ada di file **CARA MAIN.txt**.
> Kebutuhan: Windows 10/11 dengan kartu grafis yang mendukung OpenGL 3.3.
>
> Terima kasih, Pak.

## 5. (Opsional) Publikasi di itch.io

[itch.io](https://itch.io) adalah situs gratis untuk membagikan gim indie. Kelebihannya dibanding Drive: ada halaman gim (judul, deskripsi, screenshot, kontrol) sehingga lebih rapi untuk presentasi/portofolio. File yang di-upload tetap **zip yang sama**.

> Tampilan situs itch.io bisa berubah; nama tombol di bawah sesuai tampilan umum saat dokumen ini ditulis.

1. **Daftar/Login** di itch.io (disarankan akun tim).
2. Klik ikon akun (pojok kanan atas) → **Upload new project**.
3. Isi formulir:
   | Kolom | Isi |
   |---|---|
   | Title | `Burn The Wo` |
   | Project URL | mis. `burn-the-wo` |
   | Short description | mis. "Arcade 2D pixel art tentang memadamkan kebakaran hutan, dengan satire dan dark humor." |
   | Classification | **Games** |
   | Kind of project | **Downloadable** (bukan HTML: gim kita tidak berjalan di browser) |
   | Pricing | **No payments** (atau "$0 or donate") |
4. Bagian **Uploads** → **Upload files** → pilih zip dari `dist\`. Setelah ter-upload, centang platform **Windows** pada file itu.
5. **Details**: isi deskripsi (bisa salin bagian "Kontrol" dan "Cara menjalankan" dari `CARA MAIN.txt`), **Genre** (mis. Action/Simulation), **Tags** (mis. `pixel-art`, `2d`, `satire`, `arcade`).
6. **Screenshots**: upload 2–4 screenshot gameplay (Win+Shift+S saat gim berjalan). Opsional: **Cover image** 630×500.
7. **Visibility & access**, pilih:
   - **Draft**: hanya kalian yang bisa lihat (untuk cek tampilan dulu).
   - **Restricted**: hanya orang tertentu, mis. pakai password yang diberikan ke dosen. Cocok selama gim belum final.
   - **Public**: siapa saja bisa menemukan dan mendownload.
8. Klik **Save & view page**, buka halaman gim, coba tombol **Download** sendiri.
9. Kirim link halaman itch.io ke dosen dengan template pesan di atas.

Untuk versi baru: buka halaman **Edit game** → bagian Uploads → upload zip baru (centang Windows) → hapus atau sembunyikan zip lama → **Save**.

## 6. Hal yang perlu diingat

- Versi web/HTML5 **tidak** tersedia: butuh compile ulang ke WebAssembly (Emscripten) dan penyesuaian kode. Hanya dipertimbangkan setelah Tahap 7 jika waktu longgar (lihat `01-project-overview.md`).
- Saat aset (gambar/suara) mulai dipakai, engine harus mencari folder `assets\` relatif ke lokasi `.exe`, bukan relatif ke folder tempat gim dijalankan. Ini dikerjakan bersamaan dengan loader aset.
- Zip demo Tahap 1 masih memakai grafik placeholder. Pertimbangkan mengirim ke dosen setelah tahap yang lebih lengkap, kecuali diminta.
