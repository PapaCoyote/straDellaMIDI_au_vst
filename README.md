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

1. After a successful Release build, rescan plugins:  
   **Logic Pro → Preferences → Plug-In Manager → Reset & Rescan Selection**.
2. Create a **Software Instrument** track with any instrument you like.
3. In the **MIDI FX** slot of that instrument channel, insert  
   **Audio Units MIDI Effects → yourcompany → straDellaMIDI_plugin**.
4. Open the plugin window — you will see the Stradella bass grid.
5. Click any button to send MIDI notes to the instrument below it.

## Plugin Metadata

| Property | Value |
|----------|-------|
| Name | straDellaMIDI_plugin |
| Manufacturer | yourcompany |
| Manufacturer Code | Manu |
| Plugin Code | Vb4d |
| AU Type | `aumf` (MIDI Effect) |
| Bundle ID | com.yourcompany.straDellaMIDI_plugin |
| VST3 Category | Fx\|MIDI |
| Formats | AU, VST3 |

> **Before distribution:** replace the placeholder `yourcompany` / `www.yourcompany.com` / `com.yourcompany` values in `straDellaMIDI_plugin.jucer` and `JuceLibraryCode/JucePluginDefines.h` with your actual company name and reverse-domain bundle identifier, then re-save the project in Projucer to regenerate the Xcode build files.
