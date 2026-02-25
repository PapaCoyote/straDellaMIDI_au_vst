# straDellaMIDI_au_vst
Accordion Stradella Bass Side Emulator — AU (MIDI Effect) and VST3

## What It Does

**straDellaMIDI_plugin** emulates the left-hand (**Stradella bass**) side of a button-accordion.  
Clicking a button in the plugin's GUI sends the corresponding MIDI note(s) to the downstream  
instrument or DAW track.

### Layout

The GUI shows a **12-column × 6-row** grid:

| Row | Name | Notes sent |
|-----|------|------------|
| 0 | **Counterbass** | Root + perfect 5th (single note) |
| 1 | **Bass** | Root note only |
| 2 | **Major** | Root · Major-3rd · 5th |
| 3 | **Minor** | Root · Minor-3rd · 5th |
| 4 | **Dom 7** | Root · Major-3rd · 5th · Minor-7th |
| 5 | **Dim 7** | Root · Minor-3rd · Dim-5th · Dim-7th |

The 12 columns are the 12 chromatic pitches arranged in **circle-of-fifths order**:  
`Bb → F → C → G → D → A → E → B → F# → Db → Ab → Eb`

Bass notes are voiced in octave 2; chord tones are voiced one octave higher.

## Repository Contents

| Path | Description |
|------|-------------|
| `straDellaMIDI_plugin.jucer` | Projucer project file — open this in the Projucer to generate the Xcode project |
| `Source/` | Plugin C++ source files (PluginProcessor and PluginEditor) |
| `JuceLibraryCode/` | Auto-generated JUCE module wrapper files (do not edit manually) |

## Prerequisites

1. **JUCE** — download version **7.0.5 or later** from [juce.com](https://juce.com/get-juce/). JUCE 7.x is required.
2. **Projucer** — included with JUCE. Used to generate the Xcode project from the `.jucer` file.
3. **Xcode** — version 14 or later recommended (macOS only).
4. **macOS** — 10.13 (High Sierra) or later (deployment target matches the `.jucer` settings).

## Build Instructions

1. Install JUCE and note the path to the JUCE modules folder (e.g. `~/JUCE/modules`).
2. Open `straDellaMIDI_plugin.jucer` in the Projucer.
3. In the Projucer, set the **Global Paths** so that "Path to JUCE modules" points to your local JUCE modules directory.
4. Save the project in Projucer — this generates `Builds/MacOSX/straDellaMIDI_plugin.xcodeproj`.
5. Open the generated Xcode project and build the **AU** or **VST3** target (Debug or Release).
6. The built `.component` (AU) and `.vst3` files will be placed in the appropriate plugin directories by Xcode.

## Using in Logic Pro

This plugin is a **MIDI Processor** (`kAudioUnitType_MIDIProcessor`).  It appears in Logic Pro's
**MIDI FX** slot — **not** in Audio FX or Instruments.

### First-time setup / after reinstalling

1. After a successful Release build, force Logic Pro to rescan the plugin:
   * **Logic Pro → Plug-in Manager** (Logic Pro 10.7+)  
     *or* **Logic Pro → Preferences → Plug-In Manager** (older versions)  
   * Click **Reset & Rescan All** (or find the plugin in the list and click **Reset & Rescan**).
   
   If the plugin does not appear in the list at all, manually clear the AU cache first:

   ```bash
   # Quit Logic Pro first, then run:
   rm -rf ~/Library/Caches/AudioUnitCache
   ```

   Relaunch Logic Pro — it will perform a fresh scan of all components.

### Loading the plugin

1. Create or select a **Software Instrument** track (or **External MIDI** track).
2. Click the **MIDI FX** area in the channel strip (above the instrument slot).
3. Navigate to **Audio Units MIDI Effects → Papa Coyote → straDellaMIDI_1.01**.
4. Open the plugin window — you will see the Stradella bass grid.
5. Click any button to send MIDI notes to the instrument below it.

> **Note:** Because this is a MIDI FX plugin, it will **not** appear under Audio FX, Instruments,
> or in the instrument slot. If you do not see a MIDI FX slot on the channel strip, make sure you
> are looking at a **Software Instrument** channel, and that the MIDI FX lane is visible
> (click the triangular disclosure button on the channel strip if needed).

## Plugin Metadata

| Property | Value |
|----------|-------|
| Name | straDellaMIDI_1.01 |
| Manufacturer | Papa Coyote |
| Manufacturer Code | Manu |
| Plugin Code | Vb4d |
| AU Type | `aump` (MIDI Processor — Logic MIDI FX slot) |
| Bundle ID | net.papacoyote.straDellaMIDI_plugin |
| VST3 Category | Fx\|MIDI |
| Formats | AU, VST3 |
