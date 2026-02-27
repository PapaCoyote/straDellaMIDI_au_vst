#pragma once

#include <JuceHeader.h>
#include "MouseMidiExpression.h"
#include "PluginProcessor.h"

//==============================================================================
/**
    Settings window for configuring mouse MIDI expression behaviour and chord
    voicing.  Allows the user to enable/disable CC1/CC11, select the response
    curve, choose per-row octave offsets, chord inversions, and toggle the
    left/right mouse button chord extensions.
*/
class MouseMidiSettingsWindow : public juce::Component
{
public:
    //==============================================================================
    MouseMidiSettingsWindow (MouseMidiExpression& midiExpression,
                             StraDellaMIDI_pluginAudioProcessor& processor);
    ~MouseMidiSettingsWindow() override;

    void paint  (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    MouseMidiExpression&               mouseMidiExpression;
    StraDellaMIDI_pluginAudioProcessor& audioProcessor;

    // ── Section headers ───────────────────────────────────────────────────────
    juce::Label titleLabel;
    juce::Label expressionSectionLabel;
    juce::Label voicingSectionLabel;
    juce::Label majorRowLabel;
    juce::Label minorRowLabel;

    // ── Mouse Expression section ─────────────────────────────────────────────
    juce::ToggleButton modulationCheckbox;
    juce::Label        modulationLabel;

    juce::ToggleButton expressionCheckbox;
    juce::Label        expressionLabel;

    juce::ToggleButton retriggerCheckbox;
    juce::Label        retriggerLabel;

    juce::ComboBox curveSelector;
    juce::Label    curveLabel;

    // ── Voicing section ───────────────────────────────────────────────────────
    // Per-row octave offset selectors
    juce::ComboBox thirdOctaveBox;
    juce::ComboBox bassOctaveBox;
    juce::ComboBox majorOctaveBox;
    juce::ComboBox minorOctaveBox;
    juce::Label    thirdOctLabel, bassOctLabel, majorOctLabel, minorOctLabel;

    // Major row voicing
    juce::ComboBox     majorInversionBox;
    juce::Label        majorInvLabel;
    juce::ToggleButton majorLmbToggle;
    juce::Label        majorLmbLabel;
    juce::ToggleButton majorRmbToggle;
    juce::Label        majorRmbLabel;

    // Minor row voicing
    juce::ComboBox     minorInversionBox;
    juce::Label        minorInvLabel;
    juce::ToggleButton minorLmbToggle;
    juce::Label        minorLmbLabel;
    juce::ToggleButton minorRmbToggle;
    juce::Label        minorRmbLabel;

    juce::TextButton closeButton;

    //==============================================================================
    void setupUI();

    // Helpers to build the octave and inversion combo-boxes consistently.
    static void populateOctaveBox    (juce::ComboBox& box, int currentOffset);
    static void populateInversionBox (juce::ComboBox& box, int currentInversion);
    static int  octaveBoxToOffset    (int selectedId);  // ComboBox ID → offset
    static int  offsetToOctaveBoxId  (int offset);      // offset → ComboBox ID

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseMidiSettingsWindow)
};
