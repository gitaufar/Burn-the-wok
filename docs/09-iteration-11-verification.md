# Iterasi 11 — Implementasi dan verifikasi

Branch implementasi `feat/iteration-11-ui-audio-balancing` dibuat dari `main` commit `0431f95`.

## Fitur

- Esc membuka dan menutup panel pause. Tombol Lanjutkan, Ulangi Stage dan Kembali ke Menu mendukung klik atau panah/W/S dan Enter/Spasi. Timer, pemain, air, entity dan AI berhenti selama pause. Musik dan ambience loop berhenti pada posisi playback saat ini, sementara suara tombol tetap dapat diputar.
- Stage yang berhasil mendapat 1 bintang. Untuk 2 bintang, minimal 85% hutan tersisa dan minimal 75% seluruh satwa selamat. Untuk 3 bintang, minimal 95% hutan tersisa dan seluruh satwa selamat. Stage gagal mendapat 0 bintang. Ambang ini merupakan keputusan implementasi awal yang perlu dinilai saat playtest.
- Bintang muncul berurutan pada laporan hasil, masing-masing dengan suara. Jumlah seluruh satwa diteruskan lewat summary hasil agar rating menggunakan populasi stage, terpisah dari target minimum menang.
- SFX satwa panik per spesies, selamat dan terbakar dipicu pada transisi status. SFX penyalaan dan penanaman mengikuti tindakan sabotase. Boss mendapat suara saat memasuki Stage 3.
- Audio, pencarian varian SFX dan narasi intro memakai resolver yang memprioritaskan lokasi executable, dengan fallback folder kerja. Parameter gerak satwa, paparan api, dan efek hadangan sekarang terpusat di GameConfig.h.

## Verifikasi pada paket asal (catatan dari ZIP)

Harness lokal menjalankan scene dan renderer asli pada OpenGL offscreen Linux. Loader gambar Windows WIC diganti adapter khusus harness yang membaca piksel PNG asli yang sudah dikonversi; GLEW menerima konteks offscreen tanpa GLX. Adapter hanya berada di scratch dan tidak masuk kode produksi.

Uji mencakup ambang bintang, pause selama 600 fixed update tanpa perubahan waktu/air/posisi/jumlah entity, resume, retry stage, kembali ke menu, render hasil dengan bintang, dan pemuatan intro serta WAV dari folder kerja di luar repo. Screenshot pause berasal dari stage berjalan. Screenshot rating memakai summary uji (98% hutan, dua satwa selamat) untuk memeriksa layout dan animasi rating; screenshot tersebut bukan bukti kemenangan dari sesi pemain.

## Audit yang masih terbuka

- Build Debug dan Release x86 di Windows MSVC v145 perlu dijalankan. Build Linux memakai adapter uji sehingga tidak memverifikasi loader WIC atau linker MSVC.
- Uji audio dummy memeriksa pemuatan dan pemanggilan mixer, sementara volume relatif, kejelasan dan timing SFX perlu didengarkan pada perangkat audio nyata.
- Parameter stage sebelumnya dipertahankan sebagai baseline. Belum ada playtest manusia atau revisi balancing berbasis masukan pemain. Uji seluruh stage dari awal sampai ending dan catat waktu selesai, penyebab kalah dan masukan.
- Paket ZIP rilis perlu diuji dari shortcut dan laptop anggota lain. Iterasi 12 mencakup QA paket, kontrol debug dan penyelesaian kredit aset.

## Integrasi lokal Linux — 20/11/2026

Patch diterapkan pada branch `feat/iteration-11-ui-audio-balancing` berbasis `0431f95`, dengan perubahan setup Linux lokal yang sudah ada. Catatan verifikasi paket asal di atas dipertahankan sebagai riwayat; hasil berikut berasal dari pengujian integrasi sesi ini.

- Resolver aset dipindahkan dari implementasi inline yang meng-include SDL pada header publik ke `Engine/src/core/AssetPath.cpp`. Game tidak memerlukan header SDL. Source baru terdaftar pada CMake dan proyek MSVC. Loader gambar, audio, varian SFX, dan narasi memakai resolver yang sama.
- Build Debug dan Release Linux berhasil dengan GCC 13.3/CMake 3.28, SDL2/GLEW/libpng sistem dan GPU AMD Radeon/OpenGL 4.6. Pengujian memakai objek produksi Engine dan Game serta loader libpng asli; tidak memakai adapter gambar/offscreen.
- Harness integrasi lokal lulus **79 pemeriksaan**: batas rating 0/1/2/3 bintang; pause selama 600 update pada masing-masing dari tiga stage tanpa perubahan timer, posisi pemain/satwa, air, jumlah entity, atau revisi peta; resume; retry stage yang sama dengan timer/air baru; kembali ke menu; dan aksi mouse pada ketiga tombol pause.
- Diuji urutan kemunculan 1–3 bintang, lanjut ke stage berikutnya, retry setelah kalah, dan hasil stage terakhir menuju menu. Screenshot rating memakai summary fixture 98% hutan dan 2 satwa selamat; bukan kemenangan sesi pemain.
- Dari working directory di luar repo, narasi, PNG, WAV, dan varian SFX berhasil dimuat. File narasi pengecoh pada working directory tidak menimpa aset dekat executable. Path absolut dan fallback working directory juga diperiksa.
- Audio dummy memverifikasi posisi playback loop membeku saat pause, one-shot UI tetap menghasilkan sampel, dan loop lanjut dari posisi yang sama setelah resume. Ini tidak menggantikan penilaian volume/timing dengan mendengarkan langsung.
- Aplikasi desktop lokal diperbarui ke build Release Iterasi 11 dengan aset dan petunjuk main. Shortcut Desktop dan menu aplikasi yang sudah ada dipertahankan.

Build Windows MSVC, pengujian paket Windows pada laptop lain, serta playtest manusia untuk balancing tetap belum dilakukan. Parameter stage tetap memakai baseline pada patch; tidak ada klaim tuning berdasarkan masukan pemain.

Screenshot terbaru dan daftar implementasi: [ringkasan Iterasi 11](10-iteration-11-summary.md).
