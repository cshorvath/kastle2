# Kastle 2 Talking Synth (0.1)

A custom monophonic SAM speech instrument for Kastle 2 and Citadel. It uses the
Wave Bard application controls and shared hardware framework, with a streaming
speech voice instead of sample decks. Existing Wave Bard sources are unchanged.
Application EEPROM ID: **0x71**.

## Build

Requirements: Git, Python 3, CMake >=3.17, GNU Make, native C/C++ compilers,
ARM GCC **14.2.1 20241119** or later (with newlib/C++ runtime), and Pico SDK
**2.1.1**, including its pinned TinyUSB submodule. The SDK builds its own native
picotool for UF2 generation. No SDL or other speech dependencies are needed.

From the Kastle 2 checkout:

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
export PICO_TOOLCHAIN_PATH=/path/to/arm-toolchain/bin
export PATH="$PICO_TOOLCHAIN_PATH:$PATH"
cmake -S code -B code/build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build code/build --target talking-synth-with-presets --parallel 5
```

In the prepared cloud machine, first run:

```sh
source /workspace/.kastle2-tools/activate.sh
cd /workspace/kastle2
```

`PRESETS.bin` includes a small original factory phrase bank. To regenerate it or
use your own JSON banks, from `kastle2-webapps/talking-synth-editor`:

```sh
npm ci
npm run presets -- defaults ../../kastle2/code/src/apps/TalkingSynth/PRESETS.bin
# Or: npm run presets -- my-banks.json ../../kastle2/code/src/apps/TalkingSynth/PRESETS.bin
```

Rebuild `talking-synth-with-presets` after changing the binary. The existing CMake
merge helper fills the gap between code and presets; the browser emits sparse
UF2 blocks without that gap. Both write identical firmware/preset bytes.

Outputs under `code/build/output`:

- `kastle2-talking-synth.uf2`: firmware only, suitable for selecting in the editor.
- `kastle2-talking-synth-with-presets.uf2`: firmware plus factory/user phrase banks.
- `kastle2-talking-synth.elf`: debugging symbols; corresponding BIN/HEX are also built.

Power off, hold SHIFT and power on to enter the RP2040 USB bootloader. Copy the
**with-presets** UF2 to `RPI-RP2`. The firmware-only image alone does not install
presets. Missing or corrupt presets produce the framework-style alternating red
LED error indication. Flashing replaces the existing application; switching app
ID resets the app-specific EEPROM settings through the framework.

## Controls and hardware behavior

The Talking Synth application retains Wave Bard's FancyPot/FancyMode mappings,
DirtyInputsHandler synchronization, thresholds, scale quantizer, rhythmic trigger
sequencer, clock handling, MIDI input/output and output-note timing. In particular:

| Control | Behavior |
|---|---|
| SAMPLE / sample CV | Selects a phrase slot, with the existing wrapping and sticky mapping. |
| SAMPLE MOD | Left half selects without retriggering; right half triggers when the selected slot changes. |
| BANK / bank CV | Selects a phrase bank with the existing bank attenuator, mode layers, MIDI and EEPROM behavior. |
| PITCH / NOTE / free pitch CV | Controls speech fundamental pitch, with Wave Bard quantization, fine pitch, scales and root. |
| LENGTH / LENGTH MOD / envelope CV | Uses the Wave Bard attack/hold/decay curves to shape amplitude. Left half swells; right half decays. It never reverses speech or changes speaking speed. |
| TRIGGER / SHIFT / MIDI / sequencer | Interrupts the current speech voice and starts the selected phrase from its beginning, with a fresh envelope. |
| FX / FILTER / audio routing | Retains distortion, delay/chorus, DJ filter and input audio routing. |

Low MIDI notes select phrase slots using the inherited twelve-note repetition;
higher notes control pitch. Existing clock, LFO mapping, ENV output, bank LEDs,
audio codec, calibration/test mode and hardware variant detection remain in the
framework. The version announcement identifies this custom build as “draft 0.1”.

Every *committed* trigger clears the voice buffer and cancels the previous SAM
render. Wave Bard's 20 ms retrigger gate, dual-deck crossfade and reverse-trigger
suppression have been removed. Input edges still pass through the framework's
ADC/control synchronization and audio block scheduling. A new phrase has a small
text-to-phoneme startup latency; this is not a promise of zero-latency audio.
Delay/chorus feedback tails remain effects, rather than additional speech voices.

Each preset stores text, speed, mouth, throat and sing mode. Higher SAM speed
values mean **slower** speech. Pitch changes the glottal period, not the read
rate, frame speed or preset speed. Sing mode removes SAM's natural pitch contour.
The 8-bit period limits/clamps pitch; the supported relative multiplier is
0.25–4, including CV/MIDI modulation. This is a speech synthesizer, not an
unlimited-range musical oscillator.

## Architecture and limits

- Core 1 owns SAM's non-reentrant state and streams 8-bit mono PCM at 22050 Hz.
  No complete phrase buffer is allocated. Core 0 consumes a 512-byte bounded
  ring and converts it to the framework's **44000 Hz** stereo output (dual mono
  before stereo effects). This simple sample-and-hold conversion retains SAM's
  low fidelity character.
- Spinlock-backed critical sections protect the ring, pitch and trigger
  generations. No synthesis or blocking ring wait occurs inside the audio IRQ.
  Retriggers clear buffered audio and the held output sample. Cancellation also
  works when the producer is waiting on a full ring.
- Pitch updates can take up to roughly 23 ms to pass through already-rendered
  audio. SAM setup/reciter runs between phrases; its timing needs measurement on
  actual hardware. Underruns output silence rather than stale samples;
  `SpeechVoice::underruns` and `errors` can be inspected through a debugger.
- Effects run on core 0 because core 1 is now the speech producer. The original
  22000-sample stereo delay allocations remain. Worst-case IRQ CPU usage and
  heap/stack headroom must be measured on hardware before live performance use.
- Flash layout: firmware `[0x10000000,0x10080000)`; presets
  `[0x10080000,0x10800000)`. The linker enforces the 512 KB firmware limit.
- Up to 32 banks, 32 phrase slots per bank, 32 scales and 64 rhythms. Banks have
  equal slot counts to retain Wave Bard's selection semantics. Text is limited
  to 63 English ASCII characters; SAM does not provide multilingual speech.
  The legacy phoneme buffer is bounded; excessively expanding input can fail
  synthesis safely (see the error counter). MIDI/SysEx does not rewrite presets;
  use the editor and reflash.

## Tests

From the companion editor, after building firmware:

```sh
cd /workspace/kastle2-webapps/shared/bastl-react && npm ci
cd /workspace/kastle2-webapps/talking-synth-editor && npm ci
PRESET_FIXTURE=/tmp/talking-preset-fixture.bin npm test
cd /workspace/kastle2
PRESET_FIXTURE=/tmp/talking-preset-fixture.bin bash code/tests/talking-synth/run.sh
```

The firmware host tests use ASan/UBSan for speech, repeated calls, cancellation,
sing-mode reset, pitch/speed independence, a varied English phrase corpus,
concurrent ring cancellation/restart, and the JS-to-firmware binary contract.
The mocked Pico lock is a host mutex, so these are not RP2040 timing tests.
Editor tests check JSON limits, binary round trips/CRC, malformed data and actual
firmware UF2 merging. Its optional Chromium smoke check covers the visible user
flow and asserts that no audio context is created. See the editor README.

The firmware build and all these host/browser checks were run in cloud. Physical
flashing, audio quality, realtime CPU load, MIDI/CV/clock calibration, and Citadel
hardware behavior have **not** been tested on a physical device. Use an actual
Kastle 2 for those checks; a successful UF2 build is not hardware certification.

See [PRESET_FORMAT.md](PRESET_FORMAT.md) and [THIRD_PARTY.md](THIRD_PARTY.md).
