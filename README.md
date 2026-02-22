# straDellaMIDI_au_vst
Accordion stradella bass side emulator in AU and VST3 format

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

## Validating in Logic Pro

After a successful Release build:
1. Rescan plugins in Logic Pro via **Logic Pro → Preferences → Plug-In Manager → Reset & Rescan Selection**.
2. The plugin should appear under **Audio Units → yourcompany → straDellaMIDI_plugin**.

## Plugin Metadata

| Property | Value |
|----------|-------|
| Name | straDellaMIDI_plugin |
| Manufacturer | yourcompany |
| Manufacturer Code | Manu |
| Plugin Code | Vb4d |
| AU Type | aufx (audio effect) |
| Bundle ID | com.yourcompany.straDellaMIDI_plugin |
| VST3 Category | Fx |
| Formats | AU, VST3 |

> **Before distribution:** replace the placeholder `yourcompany` / `www.yourcompany.com` / `com.yourcompany` values in `straDellaMIDI_plugin.jucer` and `JuceLibraryCode/JucePluginDefines.h` with your actual company name and reverse-domain bundle identifier, then re-save the project in Projucer to regenerate the Xcode build files.
