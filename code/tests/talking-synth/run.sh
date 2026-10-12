#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../../src/apps/TalkingSynth" && pwd)
build=${TMPDIR:-/tmp}/kastle2-talking-tests
mkdir -p "$build"
cc -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer -I"$root/sam" \
  "$root"/sam/{sam,render,reciter,debug,stream}.c "$(dirname "$0")/engine.c" -o "$build/engine"
"$build/engine"
for source in sam render reciter debug stream; do
  cc -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer -I"$root/sam" -c "$root/sam/$source.c" -o "$build/$source.o"
done
c++ -std=c++17 -pthread -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I"$root" -I"$(dirname "$0")/mock" "$(dirname "$0")/voice.cpp" "$build"/*.o -o "$build/voice"
"$build/voice"
if [ -n "${PRESET_FIXTURE:-}" ]; then
  c++ -std=c++17 -g -O1 -fsanitize=address,undefined -I"$root" "$(dirname "$0")/presets.cpp" -o "$build/presets"
  "$build/presets" "$PRESET_FIXTURE"
fi
