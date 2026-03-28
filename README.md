# straDellaMIDI_au_vst
Accordion Stradella Bass Side Emulator — AU (MIDI Effect) and VST3

## What It Does

**straDellaMIDI_plugin** emulates the left-hand (**Stradella bass**) side of a button-accordion.  
Clicking a button in the plugin's GUI sends the corresponding MIDI note(s) to the downstream  
instrument or DAW track.

### Layout

The GUI shows a **12-column × 4-row** grid:

| Row | Name | Notes sent |
|-----|------|------------|
| 0 | **Counterbass** | Major 3rd above root |
| 1 | **Bass** | Root note only |
| 2 | **Major** | Root · Major-3rd · 5th |
| 3 | **Minor** | Root · Minor-3rd · 5th |

The 12 columns are the 12 chromatic pitches arranged in **circle-of-fifths order**:  
`Eb → Bb → F → C → G → D → A → E → B → F# → Db → Ab`

## Repository Contents

| Path | Description |
|------|-------------|
| `CMakeLists.txt` | **Primary build file** — use with CMake (recommended, no Projucer needed) |
| `straDellaMIDI_plugin.jucer` | Projucer project file — alternative build path |
| `Source/` | Plugin C++ source files |
| `JuceLibraryCode/` | Auto-generated JUCE module wrapper files |

## Prerequisites

* **macOS 11.0 (Big Sur) or later** — the AU format uses `kAudioUnitType_MIDIProcessor` (`aump`)  
  which requires macOS 11 at runtime, and the JUCE 8 AU wrapper uses macOS-11 APIs at compile time.
* **Xcode 14 or later** — for both the CMake and Projucer build paths.
* **JUCE 8.x** — download from [juce.com](https://juce.com/get-juce/).
* **CMake 3.22 or later** — for the CMake build path (recommended).

---

## Build — CMake (recommended)

The CMake path is the **recommended** and most reliable way to build both the AU and VST3
targets. It hard-codes the correct macOS 11.0 deployment target and all AU settings; no
Projucer is needed.

### 1. Configure

```bash
# Replace ~/JUCE with the actual path to your JUCE 8 installation.
cmake -B build -DJUCE_PATH=~/JUCE
```

Optional: to have the plugins copied to `~/Library/Audio/Plug-Ins/` automatically after
each build, add `-DCOPY_PLUGIN_AFTER_BUILD=TRUE`.

### 2. Build (Release)

```bash
cmake --build build --config Release
```

This produces two bundles inside `build/straDellaMIDI_plugin_artefacts/Release/`:

| Bundle | Install location |
|--------|-----------------|
| `AU/straDellaMIDI_plugin.component` | `~/Library/Audio/Plug-Ins/Components/` |
| `VST3/straDellaMIDI_plugin.vst3` | `~/Library/Audio/Plug-Ins/VST3/` |

### 3. Install manually (if you did not use COPY_PLUGIN_AFTER_BUILD)

```bash
# AU
cp -R build/straDellaMIDI_plugin_artefacts/Release/AU/straDellaMIDI_plugin.component \
      ~/Library/Audio/Plug-Ins/Components/

# VST3
cp -R build/straDellaMIDI_plugin_artefacts/Release/VST3/straDellaMIDI_plugin.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/
```

### Open in Xcode (optional)

```bash
cmake -B build -DJUCE_PATH=~/JUCE -GXcode
open build/straDellaMIDI.xcodeproj
```

Select the **straDellaMIDI_plugin_AU** or **straDellaMIDI_plugin_VST3** scheme and build.

---

## Build — Projucer (alternative)

> **Note:** If you are using this path and the AU does not build, switch to the CMake path
> above — it is less error-prone.

### Module paths — how they work after this fix

All modules in the `.jucer` now use **`useGlobalPath="0"`**, which means Projucer uses the
committed relative path `../modules` (relative to the `.jucer` file).  This path resolves to
the JUCE `modules/` directory for the standard install layout where this project lives
_inside_ the JUCE folder (e.g. `~/JUCE/straDellaMIDI_au_vst/`):

```
~/JUCE/                          ← JUCE root
  modules/                       ← JUCE modules ← resolved by ../modules
  straDellaMIDI_au_vst/          ← this project
    straDellaMIDI_plugin.jucer
```

If your JUCE is in a **different location**, fix the paths once in Projucer:

1. Open `straDellaMIDI_plugin.jucer` in Projucer.
2. In the left-hand **Modules** panel, click each module that shows a warning icon.
3. Uncheck "Use global path" and type the correct path to your JUCE `modules/` folder.
4. **File → Save Project** (⌘S) — your paths are now committed to the `.jucer`.

> Because `useGlobalPath="0"` is now committed, this path change **survives git pulls**
> — you should only need to do it once per machine.

### ⚠️ If you see "modules not found" (Projucer global path not configured)

If you never needed to set per-module paths before (i.e. Projucer used to find them
automatically), your Projucer global JUCE path is not configured.  Either:

- **Option A (preferred):** set the global path once — menu bar → **Projucer → Global Paths…**,
  set "Path to JUCE" to your JUCE root (e.g. `~/JUCE`).  Then re-open the `.jucer`.
- **Option B:** follow the three steps above ("fix the paths once in Projucer").

### Steps

> **Every time you clone the repo or pull changes you must open the `.jucer` in Projucer
> and save it before building.** Projucer re-generates the Xcode project (deployment
> target, AU type, module include paths, etc.) from the `.jucer` file. Skipping this
> step is the most common cause of AU compilation failures.

1. Open `straDellaMIDI_plugin.jucer` in Projucer.
2. If any module shows a warning, fix its path (see above) then re-open.
3. **File → Save Project** (⌘S) — this regenerates `Builds/MacOSX/straDellaMIDI_plugin.xcodeproj`.
4. In Xcode: **Product → Clean Build Folder** (⌥⇧⌘K).
5. Select the **straDellaMIDI_plugin - AU** or **straDellaMIDI_plugin - VST3** scheme and build
   in **Release** mode.

---

## Using in Logic Pro

This plugin is a **MIDI Processor** (`kAudioUnitType_MIDIProcessor`, `aump`). It appears in Logic
Pro's **MIDI FX** slot — **not** in Audio FX or Instruments.

> **macOS 11+ is required.** On macOS 10.15 or earlier the `aump` component type is not
> supported and the plugin will not appear regardless of scanning or cache clearing.

### Step 1 — Verify the AU passes validation

Before loading in Logic, confirm `auval` accepts the component:

```bash
auval -v aump Vb4d Manu
```

A passing result ends with `* * PASS * *`. Any `FAIL` or error output identifies exactly
what is wrong. Run this after each build to check the component before opening Logic.

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

> **Note:** The plugin appears under **MIDI FX**, not Audio FX or Instruments. If you do
> not see a MIDI FX slot, make sure the channel strip belongs to a **Software Instrument**
> track and that the MIDI FX lane is expanded (click the disclosure triangle if needed).

### Gatekeeper / quarantine (if the .component was copied from another machine)

Plugins downloaded or copied from another Mac may be quarantined by macOS Gatekeeper and
silently blocked. Remove the quarantine flag before scanning:

```bash
sudo xattr -rd com.apple.quarantine \
  ~/Library/Audio/Plug-Ins/Components/straDellaMIDI_plugin.component
```

---

## Using the VST3 in a DAW

After building the **VST3** target, the `.vst3` bundle is placed in
`~/Library/Audio/Plug-Ins/VST3/straDellaMIDI_plugin.vst3` (macOS default VST3 location).

- In **Ableton Live**, **Reaper**, **Bitwig**, **Reason Studio**, and other VST3-capable
  DAWs, rescan your plugin folder after the first build.
- The plugin appears in the **MIDI Effect** (or equivalent) category as
  `Papa Coyote / straDellaMIDI_1.01`.
- Insert it on a MIDI track and route MIDI output to a software instrument on another track.

> If the VST3 quarantine flag needs clearing:
> ```bash
> sudo xattr -rd com.apple.quarantine \
>   ~/Library/Audio/Plug-Ins/VST3/straDellaMIDI_plugin.vst3
> ```

---

## Troubleshooting: AU does not build

### Root cause analysis (history of this PR)

If you have been following this PR and the AU has been failing to build on every
attempt, here is what was actually happening:

| Symptom | Actual cause | Fix applied |
|---------|-------------|-------------|
| `juce_audio_processors` and `juce_audio_plugin_client` show as **missing** in Projucer | `juce_audio_processors_headless` was accidentally removed from the `.jucer` MODULES list. `juce_audio_processors` declares it as a **required** dependency in its `juce_module_info`, so Projucer refuses to load `juce_audio_processors` (and by cascade `juce_audio_plugin_client`) when the required dep is absent. | Restored `juce_audio_processors_headless` to MODULES + MODULEPATHS, recreated `JuceLibraryCode/include_juce_audio_processors_headless.cpp/.mm`, restored its `#include` in `JuceHeader.h`. |
| AU compilation availability errors (`'xxx' is only available on macOS 11.0`) | `macOSDeploymentTarget` was `"10.13"` in the original `.jucer`. `kAudioUnitType_MIDIProcessor` requires macOS 11; the JUCE 8 AU wrapper uses macOS-11 APIs. | Raised to `"11.0"` and reinforced via `xcodeExtraSettings="MACOSX_DEPLOYMENT_TARGET = 11.0;"` in each Xcode build configuration. |
| Module paths reset on every `git pull` | All modules used `useGlobalPath="1"`, ignoring the committed `path="../modules"`. Any local Projucer path fix was wiped each pull. | Changed all modules to `useGlobalPath="0"`. |
| Invalid bundle identifier warning | Bundle ID contained `_` (`straDellaMIDI_plugin`), which is illegal in a UTI. | Changed to `net.papacoyote.straDellaMIDI.plugin` throughout. |

**Did the Lessons class cause the AU failure?** No. The AU was already broken before
the Lessons feature was added (the `macOSDeploymentTarget="10.13"` pre-existing issue
existed in the repo before any Lessons work started). The Lessons C++ code compiles
cleanly for both AU and VST3 targets.

**Did the VST3 fix PR break the AU?** No. The VST3 fix PR only changed two `Font`
constructor calls (`juce::Font(...)` → `juce::Font(juce::FontOptions(...))`) in
`MouseMidiSettingsWindow.cpp` and `MappingSettingsWindow.cpp`. It made no changes to
any build configuration, and those two source files compile identically for AU and VST3.

**Should you abandon this PR?** No. All known configuration issues are now fixed.
After pulling the latest commit and regenerating the Xcode project (see Check 2 below)
the AU should build correctly. The CMake path is the most reliable option — it requires
no Projucer regeneration.

Work through these checks in order.

### Check 0 — Projucer "modules not found" / "modules missing" error

All modules now use `useGlobalPath="0"` with the committed relative path `../modules`.
For the standard JUCE install layout (project inside `~/JUCE/`), this resolves correctly
without any Projucer global-path setup.

If you still see "modules not found" after the fix:

1. The modules path doesn't point to your JUCE `modules/` folder — open Projucer's
   **Modules** tab, uncheck "Use global path" for the affected module, and type the correct
   path.  Save the project.  The fix persists across git pulls.
2. If your Projucer global JUCE path is configured, you can also fix it via
   **Projucer → Global Paths…** → set "Path to JUCE" to the root of your JUCE install.

> **Important:** "Attaching locally" via the Modules tab only sticks if `useGlobalPath` is
> off.  With the old `useGlobalPath="1"` (fixed in this commit), every git pull silently
> reset the path back to the global preference and any local attachment was lost.

### Check 1 — macOS deployment target

The AU build **requires macOS 11.0 as the deployment target**. With a lower target the
JUCE 8 AU wrapper fails with availability errors such as:

```
error: 'xxx' is only available on macOS 11.0 or newer
```

**CMake fix:** the `CMakeLists.txt` hard-codes `11.0` — no action needed.

**Projucer fix:** The `.jucer` specifies `macOSDeploymentTarget="11.0"` **and** injects
`MACOSX_DEPLOYMENT_TARGET = 11.0` directly into each Xcode build configuration via
`xcodeExtraSettings` (visible in the XCODE_MAC/CONFIGURATION elements if you open the
`.jucer` in a text editor). After re-saving in Projucer this forces the correct
deployment target in all targets — AU, VST3, and any others — regardless of Projucer
version. If you still see availability errors, **do a clean build** (Product → Clean Build
Folder in Xcode, or delete the `build/` directory for CMake) before rebuilding.

### Check 2 — Stale Xcode project (Projucer path only)

The `Builds/` directory is in `.gitignore`. Every time you pull changes you must
re-open the `.jucer` in Projucer and **save** to regenerate the Xcode project.
Building from a project generated before recent fixes is the most common cause of
"AU does not build" even after the source-code changes are applied.

### Check 3 — Wrong Xcode scheme

The Projucer-generated Xcode project has a **separate scheme for each format** —
`straDellaMIDI_plugin - AU` and `straDellaMIDI_plugin - VST3`. Building the VST3
scheme produces only the `.vst3`; it does **not** produce the `.component`. Verify
that the AU scheme is selected before building.

### Check 4 — auval rejects the component

If the `.component` bundle was built successfully but does not appear in Logic:

```bash
# Check the bundle was built and installed
ls ~/Library/Audio/Plug-Ins/Components/straDellaMIDI_plugin.component

# Validate with auval
auval -v aump Vb4d Manu
```

A `FAIL` result from `auval` will contain the exact reason. Common causes:
* **Wrong bundle identifier** — must be `net.papacoyote.straDellaMIDI.plugin`
* **Wrong AU type / subtype / manufacturer codes** — must be `aump / Vb4d / Manu`
* **Runtime crash** — run `auval -v aump Vb4d Manu 2>&1 | tee /tmp/auval.log` and
  inspect the log for stack traces.

### Check 5 — Component not in the right location

The built component must be at:

```
~/Library/Audio/Plug-Ins/Components/straDellaMIDI_plugin.component
```

With CMake this is the `AU/` sub-folder of the build artefacts directory.  
With Projucer/Xcode the AU scheme's post-build script copies it automatically.  
If the post-build script did not run (e.g. permission error), copy it manually:

```bash
cp -R \
  "Builds/MacOSX/build/Release/straDellaMIDI_plugin.component" \
  ~/Library/Audio/Plug-Ins/Components/
```

---

## Plugin Metadata

| Property | Value |
|----------|-------|
| Name | straDellaMIDI_1.01 |
| Manufacturer | Papa Coyote |
| Manufacturer Code | `Manu` |
| Plugin Code | `Vb4d` |
| AU Type | `aump` (MIDI Processor — Logic MIDI FX slot) |
| Bundle ID | `net.papacoyote.straDellaMIDI.plugin` |
| VST3 Category | `Fx\|MIDI` |
| Formats | AU, VST3 |
| macOS minimum | 11.0 (Big Sur) |

### Bundle ID migration note

The bundle identifier was corrected from `net.papacoyote.straDellaMIDI_plugin` (contained
an underscore, which is invalid in a UTI) to `net.papacoyote.straDellaMIDI.plugin`.

If you had the **old** plugin installed before this fix, macOS and Logic Pro will treat the
old and new bundle IDs as **two different plugins**.  To clean up:

1. **Remove the old .component** if present:

   ```bash
   rm -rf ~/Library/Audio/Plug-Ins/Components/straDellaMIDI_plugin.component
   ```

2. **Remove the old .vst3** if present:

   ```bash
   rm -rf ~/Library/Audio/Plug-Ins/VST3/straDellaMIDI_plugin.vst3
   ```

3. In **Logic Pro**, open **Logic → Preferences → Plug-in Manager**, find any stale
   `straDellaMIDI_1.01` entry, and click **Reset & Rescan All** after installing the
   newly-built plugin.

4. **Rebuild** the plugin from the updated source and copy the new `.component` /
   `.vst3` bundles to the install locations.
