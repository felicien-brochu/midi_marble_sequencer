
# MIDI Marble Sequencer

MIDI Marble Sequencer is a hardware + firmware project that detects marbles on IR sensor tracks and outputs MIDI over USB. The system is split into a main ESP-IDF firmware (sequencer + USB MIDI) and ESP32-based control boards (Arduino/PlatformIO) connected over I2C.

## Repository layout

- `hardware/` — CAD (SolidWorks parts/assemblies) and related manufacturing files.
- `software/`
	- `midi_marble_sequencer_main/` — main firmware (ESP-IDF) for the sequencer + USB MIDI.
	- `midi_marble_sequencer_controls_main/` — main control board firmware (PlatformIO/Arduino).
	- `midi_marble_sequencer_controls_pages/` — pages control board firmware (PlatformIO/Arduino).
	- `common_src/` — shared code used by the control board firmwares.
	- `smab_midi_mapper/` — JUCE utility app (see its README).
	- `stats/` — calibration/statistics data exports.

