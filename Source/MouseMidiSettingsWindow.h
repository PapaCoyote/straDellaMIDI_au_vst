#pragma once

#include <JuceHeader.h>
#include "MouseMidiExpression.h"
#include "PluginProcessor.h"

//==============================================================================
/**
    Settings window for configuring mouse MIDI expression behaviour.
    Allows the user to enable/disable CC1/CC11, select the response curve,
    and toggle the retrigger-on-direction-change behaviour.

    Chord voicing settings (octave, inversion, etc.) have moved to the
    Mapping settings window.
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

    // ── Section header ────────────────────────────────────────────────────────
    juce::Label titleLabel;
    juce::Label expressionSectionLabel;

    // ── Mouse Expression section ─────────────────────────────────────────────
    juce::ToggleButton modulationCheckbox;
    juce::Label        modulationLabel;

    juce::ToggleButton expressionCheckbox;
    juce::Label        expressionLabel;

    juce::ToggleButton retriggerCheckbox;
    juce::Label        retriggerLabel;

    juce::ComboBox curveSelector;
    juce::Label    curveLabel;

    juce::TextButton closeButton;

    //==============================================================================
    void setupUI();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseMidiSettingsWindow)
};
