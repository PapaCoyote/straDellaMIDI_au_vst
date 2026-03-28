# ==============================================================================
#  straDellaMIDI — Stradella Bass Accordion MIDI Effect Plugin
#  CMakeLists.txt  (JUCE 8 CMake API)
#
#  Usage:
#    cmake -B build -DJUCE_PATH=/path/to/JUCE
#    cmake --build build --config Release
#
#  The build produces two plugin bundles:
#    • straDellaMIDI_plugin.component  (AU  – Logic Pro MIDI FX slot)
#    • straDellaMIDI_plugin.vst3       (VST3 – Reason Studio, Ableton, …)
#
#  Optionally have cmake install the plugins automatically after building:
#    cmake -B build -DJUCE_PATH=… -DCOPY_PLUGIN_AFTER_BUILD=TRUE
# ==============================================================================

cmake_minimum_required(VERSION 3.22)

# macOS deployment target MUST be set before the first project() call.
# kAudioUnitType_MIDIProcessor requires macOS 11; JUCE 8 AU wrapper uses
# macOS-11 APIs, so a lower target causes Xcode availability errors.
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0"
    CACHE STRING "Minimum macOS deployment version" FORCE)

project(straDellaMIDI VERSION 1.0.1 LANGUAGES CXX)

# ------------------------------------------------------------------------------
# JUCE location
# ------------------------------------------------------------------------------
set(JUCE_PATH "" CACHE PATH
    "Absolute path to the JUCE library root (the folder that contains 'modules/').")

if(JUCE_PATH STREQUAL "")
    message(FATAL_ERROR
        "JUCE_PATH is not set.  Pass it on the cmake command line, e.g.:\n"
        "  cmake -B build -DJUCE_PATH=/path/to/JUCE\n"
        "JUCE 8.x is required.  Download from https://juce.com/get-juce/")
endif()

add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/juce" EXCLUDE_FROM_ALL)

# ------------------------------------------------------------------------------
# Optional: auto-copy plugins after build
# ------------------------------------------------------------------------------
option(COPY_PLUGIN_AFTER_BUILD
    "Copy built plugins to ~/Library/Audio/Plug-Ins/ after each build" OFF)

# ------------------------------------------------------------------------------
# Plugin target
# ------------------------------------------------------------------------------
juce_add_plugin(straDellaMIDI_plugin
    # Identity
    COMPANY_NAME              "Papa Coyote"
    COMPANY_WEBSITE           "www.papacoyote.net"
    BUNDLE_ID                 "net.papacoyote.straDellaMIDI.plugin"
    PLUGIN_MANUFACTURER_CODE  Manu       # must be exactly 4 chars, upper-case first
    PLUGIN_CODE               Vb4d       # must be exactly 4 chars

    # Formats to build
    FORMATS  AU  VST3

    # Plugin metadata
    PRODUCT_NAME              "straDellaMIDI_1.01"
    DESCRIPTION               "Stradella Bass Accordion MIDI Effect"

    # MIDI effect: no audio I/O buses
    IS_MIDI_EFFECT            TRUE
    NEEDS_MIDI_INPUT          TRUE
    NEEDS_MIDI_OUTPUT         TRUE
    IS_SYNTH                  FALSE

    # AU format settings
    AU_MAIN_TYPE              kAudioUnitType_MIDIProcessor
    AU_EXPORT_PREFIX          straDellaMIDI_pluginAU

    # VST3 format settings
    VST3_CATEGORIES           "Fx|MIDI"

    # The editor captures computer keyboard input for stradella-grid play.
    EDITOR_WANTS_KEYBOARD_FOCUS  TRUE

    # Hardened runtime / notarisation (off by default; enable for distribution)
    HARDENED_RUNTIME_ENABLED  FALSE

    COPY_PLUGIN_AFTER_BUILD   ${COPY_PLUGIN_AFTER_BUILD}
)

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------
target_sources(straDellaMIDI_plugin
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/StradellaKeyboardMapper.cpp
        Source/StradellaKeyboardMapper.h
        Source/MouseMidiExpression.cpp
        Source/MouseMidiExpression.h
        Source/MouseMidiSettingsWindow.cpp
        Source/MouseMidiSettingsWindow.h
        Source/MappingSettingsWindow.cpp
        Source/MappingSettingsWindow.h
        Source/LessonsWindow.cpp
        Source/LessonsWindow.h
)

# ------------------------------------------------------------------------------
# Compile definitions (match the .jucer JUCEOPTIONS)
# ------------------------------------------------------------------------------
target_compile_definitions(straDellaMIDI_plugin
    PUBLIC
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

# ------------------------------------------------------------------------------
# Link JUCE modules
# ------------------------------------------------------------------------------
target_link_libraries(straDellaMIDI_plugin
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
)
