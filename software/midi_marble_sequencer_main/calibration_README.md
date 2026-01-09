# Calibration Procedure

This guide explains how to calibrate the IR sensors for the MIDI Marble Sequencer.

## Overview

The calibration process measures the IR sensor response for different marble colors to establish accurate detection thresholds. This ensures reliable marble detection during operation.

## Starting Calibration Mode

1. **Power off** the device
3. **Power on** the device while **holding the calibration button** (the calibration button is at the back of the sequencer)
4. **Release** the button once the device starts

The device will enter calibration mode instead of normal operation.

## Board Calibration Phase

Before the main sensor calibration begins, you'll need to complete a board calibration phase. This phase ensures that all IR sensor boards have their potentiometers properly adjusted for optimal performance.

### What You Need
- **1 Brown marble** (for sensor 0)
- **1 Blue marble** (for sensor 15)

### How It Works

The board calibration phase uses LED feedback to help you adjust each board's potentiometer:

1. **LED Indicators:**
   - The **BeatsLEDSnake** shows which board is currently selected (one LED lit)
   - **Top LED line** (played_pages_led): Shows the brown marble reading on sensor 0
   - **Bottom LED line** (edited_pages_led): Shows the blue marble reading on sensor 15

2. **LED Feedback Pattern:**
   - **LED 0:** Value too low (below acceptable range)
   - **LEDs 1, 2, or 3:** Approaching target from below
   - **LEDs 3 and 4 (both):** Perfect! Value is on target ✓
   - **LEDs 4, 5, or 6:** Moving away from target toward upper limit
   - **LED 7:** Value too high (above acceptable range)

3. **Target Values:**
   - **Brown marble (sensor 0):** Target = 450, Acceptable range [250, 650]
   - **Blue marble (sensor 15):** Target = 2400, Acceptable range [2000, 2800]

### Step-by-Step Instructions

1. **Place the Marbles**
   - Place a **brown marble** on **sensor 0** of the selected board
   - Place a **blue marble** on **sensor 15** of the selected board

2. **Adjust the Potentiometer**
   - Watch the LED lines for feedback
   - Turn the board's potentiometer until **both LED lines show LEDs 3 and 4 lit together**
   - This indicates optimal calibration for both sensors

3. **Move to Next Board**
   - **Short press** the calibration button to select the next board
   - The BeatsLEDSnake LED will move to indicate the newly selected board
   - Sensor values for the newly selected board will be logged to console

4. **Repeat for All Boards**
   - Repeat steps 1-3 for all 16 boards
   - You can cycle through boards as many times as needed to fine-tune adjustments

5. **Complete Board Calibration**
   - Once all boards are calibrated, **press and hold** the calibration button for **5 seconds**
   - The system will wait for the button to be released, then proceed to sensor calibration

## Sensor Calibration Process

### What You Need
- **32 marbles of each color** (160 marbles total):
  - 32 Brown marbles
  - 32 Orange marbles
  - 32 Green marbles
  - 32 Blue marbles
  - 32 White marbles

### First Cycle Setup Example

Here's what the first calibration cycle looks like on your device:

```
        ┌─ Eighth Note Positions 0→31 (Left to Right)  ─┐
        │                                               │
Track 0 │ [  ][  ][  ][  ][  ][  ][  ][  ] ... [  ][  ] │ ← Empty (no marbles)
Track 1 │ [Br][Br][Br][Br][Br][Br][Br][Br] ... [Br][Br] │ ← 32 Brown marbles
Track 2 │ [Or][Or][Or][Or][Or][Or][Or][Or] ... [Or][Or] │ ← 32 Orange marbles
Track 3 │ [Gr][Gr][Gr][Gr][Gr][Gr][Gr][Gr] ... [Gr][Gr] │ ← 32 Green marbles
Track 4 │ [Bl][Bl][Bl][Bl][Bl][Bl][Bl][Bl] ... [Bl][Bl] │ ← 32 Blue marbles
Track 5 │ [Wh][Wh][Wh][Wh][Wh][Wh][Wh][Wh] ... [Wh][Wh] │ ← 32 White marbles
Track 6 │ [  ][  ][  ][  ][  ][  ][  ][  ] ... [  ][  ] │ ← Empty (no marbles)
Track 7 │ [  ][  ][  ][  ][  ][  ][  ][  ] ... [  ][  ] │ ← Empty (no marbles)
        └───────────────────────────────────────────────┘
```

In subsequent cycles, the pattern shifts up by one track (Track 1 → Track 0, Track 2 → Track 1, etc.).

### Step-by-Step Instructions

The calibration process runs 64 measurement cycles. For each cycle, place marbles according to the LEDs (or console table), start the measurement, wait for completion, then rearrange for the next cycle.

1. **Check the Current Cycle Placement**
    - Place marbles on tracks with a lit LED, leave other tracks empty
    - **Optional - Console Output:** If you have console access, it displays the exact color per track:
     ```
     |    0   |    1   |    2   |    3   |    4   |    5   |    6   |    7   |
   | NONE   | Brown  | Orange | Green  | Blue   | White  | NONE   | NONE   |
     Measure 1/64
     ```

2. **Place the Marbles**
   - For each track with a lit LED, place one marble on every sensor
   - **Color assignment pattern:** Brown → Orange → Green → Blue → White; 3 tracks are always empty
   - On the first cycle: Track 1 gets Brown, Track 2 gets Orange, Track 3 gets Green, Track 4 gets Blue, Track 5 gets White (Tracks 0, 6, 7 are empty)

3. **Confirm Placement**
   - **Press the calibration button once** when marbles are correctly placed
   - Wait for the measurement to complete - the control board display will update to show progress

4. **Repeat for All Cycles**
   - After each measurement completes, the firmware advances to the next cycle and updates the LEDs for the next arrangement. If the LED pattern did not change, rotate the marbles on the same track (see: Rotating Marbles Within the Same Track). If the LED pattern changed, Rearrange marbles to match the new LED pattern (see: Shifting Marbles Between Tracks)

### Rotating Marbles Within the Same Track

```
Before rotation (Current cycle measurement complete):
Track 0 │ [Br][Br][Br][Br][Br][Br][Br][Br] ... [Br][Br] │
          ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓         ↓   ↓
          Move all marbles one position to the left 
          The leftmost marble wraps around to the right

After rotation (Ready for next cycle):
Track 0 │ [Br][Br][Br][Br][Br][Br][Br][Br] ... [Br][Br] │
```

**Why rotate?** This ensures different individual marbles are tested at each sensor position, accounting for physical variations between marbles of the same color.

### Shifting Marbles Between Tracks

When the color pattern rotates to the next cycle, you need to shift marbles between tracks:

```
Cycle 1 - Initial setup:
Track 0 │ [  ][  ][  ][  ][  ][  ][  ][  ] ... [  ][  ] │ ← Empty
Track 1 │ [Br][Br][Br][Br][Br][Br][Br][Br] ... [Br][Br] │ ← Brown
Track 2 │ [Or][Or][Or][Or][Or][Or][Or][Or] ... [Or][Or] │ ← Orange
Track 3 │ [Gr][Gr][Gr][Gr][Gr][Gr][Gr][Gr] ... [Gr][Gr] │ ← Green
Track 4 │ [Bl][Bl][Bl][Bl][Bl][Bl][Bl][Bl] ... [Bl][Bl] │ ← Blue
Track 5 │ [Wh][Wh][Wh][Wh][Wh][Wh][Wh][Wh] ... [Wh][Wh] │ ← White
Track 6 │ [  ][  ][  ][  ][  ][  ][  ][  ] ... [  ][  ] │ ← Empty
Track 7 │ [  ][  ][  ][  ][  ][  ][  ][  ] ... [  ][  ] │ ← Empty
          ↓ Colors shift UP one track; empty tracks also shift ↓

Cycle 2 - After shifting:
Track 0 │ [Br][Br][Br][Br][Br][Br][Br][Br] ... [Br][Br] │ ← Brown (was Track 1)
Track 1 │ [Or][Or][Or][Or][Or][Or][Or][Or] ... [Or][Or] │ ← Orange (was Track 2)
Track 2 │ [Gr][Gr][Gr][Gr][Gr][Gr][Gr][Gr] ... [Gr][Gr] │ ← Green (was Track 3)
Track 3 │ [Bl][Bl][Bl][Bl][Bl][Bl][Bl][Bl] ... [Bl][Bl] │ ← Blue (was Track 4)
Track 4 │ [Wh][Wh][Wh][Wh][Wh][Wh][Wh][Wh] ... [Wh][Wh] │ ← White (was Track 5)
Track 5 │ [  ][  ][  ][  ][  ][  ][  ][  ] ... [  ][  ] │ ← Empty (was Track 6)
Track 6 │ [  ][  ][  ][  ][  ][  ][  ][  ] ... [  ][  ] │ ← Empty (was Track 7)
Track 7 │ [  ][  ][  ][  ][  ][  ][  ][  ] ... [  ][  ] │ ← Empty (was Track 0)
```

## Saving Calibration Data

After all measurements are complete:

1. **Confirm Save**
   - The console will prompt: `Save calibration data? Press button for 5 seconds to confirm.`
   - **Press and hold** the calibration button for **5 seconds** to save
   - Release early to cancel without saving

2. **Completion**
   - If saved successfully, the console will prompt: `Calibration data saved successfuly.`
   - The completion rate display will show 100%
