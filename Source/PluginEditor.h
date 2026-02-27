/*
  ==============================================================================

    StraDellaMIDI – Stradella Bass Accordion MIDI Effect Plugin
    Editor (GUI) declaration.

    The editor draws a 12-column × 4-row grid of clickable buttons that mirror
    the left-hand (Stradella bass) side of an accordion.  Clicking a button
    sends the corresponding MIDI note(s) to the plugin processor.

    Keyboard input is handled by StradellaKeyboardMapper, which maps rows of
    computer keyboard keys to accordion rows (third, bass, major, minor).

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "StradellaKeyboardMapper.h"
#include "MouseMidiExpression.h"
#include "MouseMidiSettingsWindow.h"

//==============================================================================
class StraDellaMIDI_pluginAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                                  private juce::FocusChangeListener
{
public:
    explicit StraDellaMIDI_pluginAudioProcessorEditor (StraDellaMIDI_pluginAudioProcessor&);
    ~StraDellaMIDI_pluginAudioProcessorEditor() override;

    //==============================================================================
    void paint   (juce::Graphics&) override;
    void resized ()                override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp   (const juce::MouseEvent&) override;

    bool keyPressed      (const juce::KeyPress&) override;
    bool keyStateChanged (bool isKeyDown)        override;

private:
    //==============================================================================
    // FocusChangeListener: re-asserts keyboard focus when Focus mode is active.
    void globalFocusChanged (juce::Component* focusedComponent) override;
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

    // Keyboard input: maps computer key codes to their active grid cell.
    StradellaKeyboardMapper keyboardMapper;
    juce::HashMap<int, int> activeKeyRow;   ///< keyCode → plugin row
    juce::HashMap<int, int> activeKeyCol;   ///< keyCode → plugin column

    // Mouse MIDI expression (accordion bellows emulation)
    MouseMidiExpression mouseExpression;

    // Highlight grid cells that are currently triggered by the keyboard.
    bool keyboardPressedGrid[StraDellaMIDI_pluginAudioProcessor::NUM_ROWS]
                            [StraDellaMIDI_pluginAudioProcessor::NUM_COLUMNS] {};

    // Bottom action buttons
    juce::TextButton aboutButton      { "About" };
    juce::TextButton mappingButton    { "Mapping" };
    juce::TextButton expressionButton { "Expression" };

    // Top action buttons
    juce::TextButton focusButton { "Focus" };   ///< toggle – captures keyboard & mouse focus
    juce::TextButton panicButton { "!" };       ///< sends All Notes Off on all channels
    bool             focusActive { false };     ///< mirrors focusButton toggle state

    // Original plugin size stored when Focus mode expands the window to fill screen.
    // Zero when not in full-screen focus mode.
    int originalWidth  { 0 };
    int originalHeight { 0 };

    // Layout constants (pixels)
    static constexpr int kTitleH    = 55;   // branding / title area height
    static constexpr int kHeaderH   = 30;   // column-name header height
    static constexpr int kLabelW    = 82;   // row-name label width
    static constexpr int kBtnW      = 62;   // button cell width
    static constexpr int kBtnH      = 52;   // button cell height
    static constexpr int kRowOffset = 5;    // x offset added per row for stagger
    static constexpr int kBottomH   = 40;   // bottom button area height

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StraDellaMIDI_pluginAudioProcessorEditor)
};
