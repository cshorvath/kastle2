# Source bundle

Extract talking-synth-source.zip over sibling checkouts of:

- bastl-instruments/kastle2 at a4a7a0451038bc4ed4719f46573c3eff92e7c7ce
- bastl-instruments/kastle2-webapps at 776ebac30f146b204b42217015f197877f8c4486

The archive contains the new application, vendored SAM port, host tests, new
editor and its lockfile, build/run helpers, and the two index-document additions.
It excludes toolchains, SDK/build caches, node_modules and compiled web assets.

Build/test instructions and third-party notices are in:

- kastle2/code/src/apps/TalkingSynth/README.md
- kastle2/code/src/apps/TalkingSynth/THIRD_PARTY.md
- kastle2-webapps/talking-synth-editor/README.md

The accompanying with-presets UF2 includes two default phrases. Firmware-only
UF2 is the input to the browser editor. Hardware operation is unverified.
SAM has no open-source redistribution license. These are local review/build
artifacts; their presence does not establish rights to redistribute SAM or Bastl
visuals. No release or hosted application was published.
