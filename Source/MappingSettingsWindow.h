#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
    Settings window for chord voicing.  Allows the user to choose per-row octave
    offsets, chord inversions, and toggle the left/right mouse button chord
    extensions for the Major and Minor rows.
*/
class MappingSettingsWindow : public juce::Component
{
public:
    //==============================================================================
    explicit MappingSettingsWindow (StraDellaMIDI_pluginAudioProcessor& processor);
    ~MappingSettingsWindow() override;

    void paint  (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    StraDellaMIDI_pluginAudioProcessor& audioProcessor;

    // ── Section headers ───────────────────────────────────────────────────────
    juce::Label titleLabel;
    juce::Label voicingSectionLabel;
    juce::Label majorRowLabel;
    juce::Label minorRowLabel;

    // ── Per-row octave offset selectors ──────────────────────────────────────
    juce::ComboBox thirdOctaveBox;
    juce::ComboBox bassOctaveBox;
    juce::ComboBox majorOctaveBox;
    juce::ComboBox minorOctaveBox;
    juce::Label    thirdOctLabel, bassOctLabel, majorOctLabel, minorOctLabel;

    // ── Major row voicing ─────────────────────────────────────────────────────
    juce::ComboBox     majorInversionBox;
    juce::Label        majorInvLabel;
    juce::ToggleButton majorLmbToggle;
    juce::Label        majorLmbLabel;
    juce::ToggleButton majorRmbToggle;
    juce::Label        majorRmbLabel;

    // ── Minor row voicing ─────────────────────────────────────────────────────
    juce::ComboBox     minorInversionBox;
    juce::Label        minorInvLabel;
    juce::ToggleButton minorLmbToggle;
    juce::Label        minorLmbLabel;
    juce::ToggleButton minorRmbToggle;
    juce::Label        minorRmbLabel;

    juce::TextButton closeButton;

    //==============================================================================
    void setupUI();

    static void populateOctaveBox    (juce::ComboBox& box, int currentOffset);
    static void populateInversionBox (juce::ComboBox& box, int currentInversion);
    static int  octaveBoxToOffset    (int selectedId);
    static int  offsetToOctaveBoxId  (int offset);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MappingSettingsWindow)
};
