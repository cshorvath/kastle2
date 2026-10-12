# Third-party provenance and redistribution

## Bastl firmware framework

Based on `bastl-instruments/kastle2` commit
`a4a7a0451038bc4ed4719f46573c3eff92e7c7ce`. The existing
`cshorvath/kastle2` checkout matched that upstream commit during implementation.
Wave Bard control/effects code and parameter tables retain their original MIT
notices (Marek Mach, Václav Mach / Bastl Instruments). Original application
sources were left unchanged; Talking Synth is an additional application.
The small default phrase bank contains original text and no Bastl audio samples.
Shared calibration/version recordings keep their existing CC0 provenance.

New integration, format, tests and editor code is provided under the repository's
MIT terms. **Those terms do not apply to the SAM sources below.**

## Software Automatic Mouth / s-macke/SAM

Source: <https://github.com/s-macke/SAM>, commit
`a7b36efac730957b59471a42a45fd779f94d77dd`.
Copied files are in `sam/`, except `stream.c` and `stream.h`, which are the new
MIT adapter. No SDL/CLI code is included. Port changes remove full-phrase
allocation, stream timed output, support cancellation and live glottal pitch,
reset output timing/sing mode, bound reciter expansion, avoid a sentinel table
read, and remove a legacy silent text truncation. The core phoneme/rule/formant
tables remain upstream SAM data.

The upstream repository contains **no open-source license grant**. Its README
says the code is reverse-engineered from the 1982 commercial SAM program and:

> “As long this is the case I cannot put my code under any specific open source
> software license.”

It calls the work “Abandonware” and suggests possible US fair use. Abandonware
is not a license, and that statement does not establish redistribution rights.
Do not label the complete firmware MIT/open source or assume permission to
publish SAM-derived source/binaries. Obtain permission or legal review before
redistribution. This task produced local source and firmware for review; it did
not publish releases or distribute them through a hosted editor.

## Companion web app

Architecture/shared components from `bastl-instruments/kastle2-webapps`, commit
`776ebac30f146b204b42217015f197877f8c4486`. Its code is MIT; documentation has the
repository's CC BY SA terms. Bastl/Anymade logos and graphic elements retain
**all rights reserved** status. Reusing the existing shared Header locally does
not grant rights to redistribute graphics or imply Bastl endorsement.
