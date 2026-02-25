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
4. **macOS 11.0 (Big Sur) or later** — the AU format uses `kAudioUnitType_MIDIProcessor` (`aump`),
   which requires macOS 11+. The VST3 format works on macOS 10.13+.

## Build Instructions — IMPORTANT: Projucer must be re-run first

> **Every time you clone the repo or pull changes, you must open the `.jucer` in Projucer and save
> it before building.** The `Builds/` directory (which contains the Xcode project) is not committed
> to the repository. Projucer re-generates it from the `.jucer` file, writing the correct bundle
> identifier, AU type, and other build settings into the Xcode project. Skipping this step means
> the built plugin will have stale or incorrect metadata and Logic Pro will not recognise it.

1. Install JUCE and note the path to the JUCE modules folder (e.g. `~/JUCE/modules`).
2. Open `straDellaMIDI_plugin.jucer` in the Projucer.
3. In the Projucer, set the **Global Paths** so that "Path to JUCE modules" points to your local
   JUCE modules directory.
4. **Save the project in Projucer** — this regenerates `Builds/MacOSX/straDellaMIDI_plugin.xcodeproj`
   and `JuceLibraryCode/JucePluginDefines.h`.
5. In Xcode: **Product → Clean Build Folder** (⌥⇧⌘K), then build the **AU** target in **Release**.
6. Xcode copies the built `.component` into `~/Library/Audio/Plug-Ins/Components/` automatically.

## Using in Logic Pro

This plugin is a **MIDI Processor** (`kAudioUnitType_MIDIProcessor`, `aump`). It appears in Logic
Pro's **MIDI FX** slot — **not** in Audio FX or Instruments.

> **macOS 11+ is required** for the AU format. On macOS 10.15 or earlier the `aump` component type
> is not supported and the plugin will not appear regardless of scanning or cache clearing.

### Step 1 — Verify the AU passes validation

Before loading in Logic, confirm `auval` accepts the component:

```bash
auval -v aump Vb4d Manu
```

A passing result ends with `* * PASS * *`. Any `FAIL` or error output identifies exactly what is
wrong. Run this after each Xcode build to check the component before opening Logic.

### Step 2 — Clear the AU cache and rescan

Logic caches plugin validation results. After rebuilding you must clear the stale cache:

```bash
# Quit Logic Pro first, then:
rm -rf ~/Library/Caches/AudioUnitCache
rm -rf ~/Library/Caches/com.apple.audiounits.cache 2>/dev/null; true
```

Then re-open Logic Pro. It performs a fresh scan on startup and should now find the plugin.

### Step 3 — Force a rescan inside Logic Pro

If the plugin still does not appear after clearing the cache:

* **Logic Pro → Plug-in Manager** (Logic Pro 10.7+)  
  *or* **Logic Pro → Preferences → Plug-In Manager** (older versions)  
* Locate **straDellaMIDI_1.01** in the list and click **Reset & Rescan Selection**.
* If it does not appear in the list at all, click **Reset & Rescan All**.

### Step 4 — Load the plugin

1. Create or select a **Software Instrument** track.
2. Click the **MIDI FX** slot in the channel strip (above the instrument plugin slot).
3. Navigate to **Audio Units MIDI Effects → Papa Coyote → straDellaMIDI_1.01**.
4. Open the plugin window — you will see the Stradella bass grid.
5. Click any button to send MIDI notes to the instrument below it.

> **Note:** The plugin appears under **MIDI FX**, not Audio FX or Instruments. If you do not see a
> MIDI FX slot, make sure the channel strip belongs to a **Software Instrument** track and that the
> MIDI FX lane is expanded (click the disclosure triangle on the channel strip if needed).

### Gatekeeper / quarantine (if the .component was copied from another machine)

Plugins downloaded or copied from another Mac may be quarantined by macOS Gatekeeper and silently
blocked. Remove the quarantine flag before scanning:

```bash
sudo xattr -rd com.apple.quarantine \
  ~/Library/Audio/Plug-Ins/Components/straDellaMIDI_plugin.component
```

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
