#pragma once

#include <JuceHeader.h>
#include "MouseMidiExpression.h"

//==============================================================================
/**
    Settings window for configuring mouse MIDI expression behavior.
    Allows user to enable/disable CC1 and CC11, and select curve type.
*/
class MouseMidiSettingsWindow : public juce::Component
{
public:
    //==============================================================================
    MouseMidiSettingsWindow(MouseMidiExpression& midiExpression);
    ~MouseMidiSettingsWindow() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    MouseMidiExpression& mouseMidiExpression;
    
    // UI Components
    juce::Label titleLabel;
    
    juce::ToggleButton modulationCheckbox;
    juce::Label modulationLabel;
    
    juce::ToggleButton expressionCheckbox;
    juce::Label expressionLabel;
    
    juce::ToggleButton retriggerCheckbox;
    juce::Label retriggerLabel;
    
    juce::ComboBox curveSelector;
    juce::Label curveLabel;
    
    juce::TextButton closeButton;
    
    //==============================================================================
    void setupUI();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseMidiSettingsWindow)
};
