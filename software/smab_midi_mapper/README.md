
# smab_midi_mapper

JUCE MIDI-effect plugin (VST3 / AU / Standalone) that maps “SMAB” MIDI input notes to one or more output notes/channels using mapping presets.

This folder is part of the larger MIDI Marble Sequencer repo.

## What it builds

The Projucer project is configured for:
- `VST3`
- `AU` (macOS)
- `Standalone` (useful on Windows for MIDI routing)

See `smab_midi_mapper.jucer` for the exact exporter/configuration.

## Requirements

- JUCE + Projucer
- A C++ toolchain depending on your OS/exporter
  - Windows: Visual Studio (exporters include VS2022/VS2026)
- (Optional, Windows) a virtual MIDI cable such as `loopMIDI` for routing MIDI to other apps

## Build (Projucer)

1. Open `smab_midi_mapper.jucer` in Projucer.
2. Make sure your JUCE modules path is configured in Projucer (this project uses global paths).
3. Click **Save Project** to generate the IDE project.
4. Build the target you want:
	- Standalone app (easiest to test)
	- VST3 plugin (to use inside a DAW)

Generated projects and binaries land under `Builds/`.

## Presets location (important)

Preset storage is currently a hardcoded absolute path in `Source/paths.h`:

- `MAPPING_PRESETS_PATH`

You will almost certainly need to change it to a valid folder on your machine.

## Usage (Windows example)

### Standalone (recommended for quick tests)

1. Create a virtual MIDI port with `loopMIDI` (e.g. `SMAB_Mapped_Out`).
2. Run the Standalone build.
3. In the app’s MIDI settings:
	- Input: select the MIDI Marble Sequencer (or any MIDI controller)
	- Output: select your virtual port (e.g. `SMAB_Mapped_Out`)
4. In your DAW / synth app, select `SMAB_Mapped_Out` as the MIDI input.

### VST3 (in a DAW)

1. Build the VST3.
2. Install/copy it to your system VST3 folder (or add the build folder to your DAW’s VST3 search paths).
3. Insert it as a MIDI effect on a track and route MIDI through it.

## Notes

- Default input notes are defined in `MidiMapping::getDefaultInputNotes()`.
- Mapping can fan out (one input note can trigger multiple outputs) up to `maxOutputsPerInput`.

