#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
build_dir="$repo_dir/build/linux"
build_type=Debug
action="${1:-run}"
if (( $# > 0 )); then shift; fi

case "$action" in
    run|build|release)
        if [[ "$action" == release ]]; then build_type=Release; fi
        command -v cmake >/dev/null || {
            echo 'CMake belum terpasang. Lihat docs/09-linux-setup.md.' >&2
            exit 1
        }
        cmake -S "$repo_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type"
        cmake --build "$build_dir" --parallel "${BUILD_JOBS:-4}"
        if [[ "$action" == build ]]; then exit 0; fi
        ;;
    play)
        if [[ ! -x "$build_dir/burn-the-wok" ]]; then
            echo 'Build belum ada. Jalankan ./run.sh terlebih dahulu.' >&2
            exit 1
        fi
        ;;
    help|-h|--help)
        cat <<'HELP'
Pemakaian: ./run.sh [run|build|release|play|help] [argumen game]
  run      Build Debug lalu main (default)
  build    Build Debug saja
  release  Build Release lalu main
  play     Main dengan build terakhir tanpa build ulang
  help     Tampilkan bantuan
Contoh: ./run.sh run --skip-intro
HELP
        exit 0
        ;;
    *)
        echo "Perintah tidak dikenal: $action. Jalankan ./run.sh help." >&2
        exit 2
        ;;
esac

# Audio and narration also use paths relative to the working directory.
cd -- "$build_dir"
exec ./burn-the-wok "$@"
