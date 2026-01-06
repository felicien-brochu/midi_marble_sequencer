# MIDI Marble Sequencer — firmware (main)

Compact firmware for the MIDI Marble Sequencer. Runs on ESP-IDF and exposes a USB MIDI device while controlling sensor boards, control boards, and a sequencer engine.

## Highlights
- Real-time sequencer with 8 tracks × 32 eighth-note positions
- IR sensor-based marble color detection and calibration
- USB MIDI output via TinyUSB
- Control boards over I2C (main controls + pages)
- Calibrate IR sensors via dedicated calibration mode

## Requirements
- ESP-IDF v5.x
- C/C++ toolchain supported by ESP-IDF (esp-clang / xtensa)
- USB host supporting TinyUSB (configured in project)
- Connected hardware: IR sensor boards, control boards, calibration button, LEDs

Repo uses component manifest: espressif/esp_tinyusb and idf ^5.0

## Build & Flash (typical)
1. Install ESP-IDF v5 and toolchain following Espressif instructions.
2. From firmware directory (where this README.md lives):
    - idf.py build
    - idf.py -p /dev/ttyUSB0 flash monitor
    Replace serial port as needed.

## Calibration mode
To enter calibration:
1. Power OFF the device.
2. Power ON while holding the calibration button (back of the device).
3. Release the button when the device boots into calibration mode.

See calibration_README.md for detailed step-by-step instructions and measurement patterns.

## Usage
- Plug device into a host — it appears as a MIDI device.
- Use the hardware controls (play/pause, BPM potentiometer, page buttons, rotary buttons) to control playback and editing.
- Sequencer state and controls are managed by ControlBoardsController and Sequencer classes; MIDI mapping is handled by MidiMapper.

## Project layout (main)
- main/
  - Sequencer.* — sequencer logic and page management
  - MasterClock.* — timing and scheduling
  - MarbleDetector.* — detect marbles using thresholds
  - MidiController.*, MidiMapper.* — MIDI output and mapping
  - ControlBoardsController.* — I2C communications with control boards
  - calibration/ — calibration controller, display, storage, stats
  - IRSens* — IR sensor reading utilities
  - CMakeLists.txt, idf_component.yml, README.md

## Contributing
- Open PRs against main branch.
- Keep firmware changes modular and test with hardware where possible.

## License
Add a license file to the repo (e.g., MIT) if desired.
