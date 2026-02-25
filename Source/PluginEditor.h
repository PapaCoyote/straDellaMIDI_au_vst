/*
  ==============================================================================

    StraDellaMIDI – Stradella Bass Accordion MIDI Effect Plugin
    Editor (GUI) declaration.

    The editor draws a 12-column × 6-row grid of clickable buttons that mirror
    the left-hand (Stradella bass) side of an accordion.  Clicking a button
    sends the corresponding MIDI note(s) to the plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class StraDellaMIDI_pluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit StraDellaMIDI_pluginAudioProcessorEditor (StraDellaMIDI_pluginAudioProcessor&);
    ~StraDellaMIDI_pluginAudioProcessorEditor() override;

    //==============================================================================
    void paint   (juce::Graphics&) override;
    void resized ()                override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp   (const juce::MouseEvent&) override;

private:
    //==============================================================================
    // Layout helpers
    juce::Rectangle<int> buttonBounds (int row, int col) const;
    void                 hitTest      (juce::Point<int> pos, int& rowOut, int& colOut) const;

    // Visual appearance helpers
    juce::Colour rowColour (int row, bool pressed) const;

    //==============================================================================
    StraDellaMIDI_pluginAudioProcessor& audioProcessor;

    // Track which button (if any) is currently held by the mouse.
    int pressedRow { -1 };
    int pressedCol { -1 };

    // Layout constants (pixels)
    static constexpr int kHeaderH   = 30;   // column-name header height
    static constexpr int kLabelW    = 82;   // row-name label width
    static constexpr int kBtnW      = 62;   // button cell width
    static constexpr int kBtnH      = 52;   // button cell height

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StraDellaMIDI_pluginAudioProcessorEditor)
};
