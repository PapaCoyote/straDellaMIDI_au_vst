/*
  ==============================================================================

    StraDellaMIDI – Stradella Bass Accordion MIDI Effect Plugin
    Editor (GUI) implementation.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using Proc = StraDellaMIDI_pluginAudioProcessor;

//==============================================================================
StraDellaMIDI_pluginAudioProcessorEditor::StraDellaMIDI_pluginAudioProcessorEditor (
        StraDellaMIDI_pluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Total width  = label column + 12 button columns
    // Total height = header row + 6 button rows
    const int w = kLabelW + Proc::NUM_COLUMNS * kBtnW;
    const int h = kHeaderH + Proc::NUM_ROWS   * kBtnH;
    setSize (w, h);
}

StraDellaMIDI_pluginAudioProcessorEditor::~StraDellaMIDI_pluginAudioProcessorEditor()
{
}

//==============================================================================
// Returns the pixel bounds for a given button cell.
juce::Rectangle<int>
StraDellaMIDI_pluginAudioProcessorEditor::buttonBounds (int row, int col) const
{
    const int x = kLabelW + col * kBtnW;
    const int y = kHeaderH + row * kBtnH;
    return { x, y, kBtnW, kBtnH };
}

// Converts a pixel position to a row/col index (-1 if outside the grid).
void StraDellaMIDI_pluginAudioProcessorEditor::hitTest (
        juce::Point<int> pos, int& rowOut, int& colOut) const
{
    rowOut = colOut = -1;

    const int col = (pos.x - kLabelW) / kBtnW;
    const int row = (pos.y - kHeaderH) / kBtnH;

    if (col >= 0 && col < Proc::NUM_COLUMNS &&
        row >= 0 && row < Proc::NUM_ROWS    &&
        pos.x >= kLabelW && pos.y >= kHeaderH)
    {
        rowOut = row;
        colOut = col;
    }
}

// Returns a colour for a given row, lightened when the button is pressed.
juce::Colour
StraDellaMIDI_pluginAudioProcessorEditor::rowColour (int row, bool pressed) const
{
    juce::Colour base;
    switch (row)
    {
        case Proc::COUNTERBASS: base = juce::Colour (0xffb0b0b0); break; // light grey
        case Proc::BASS:        base = juce::Colour (0xffffffff); break; // white
        case Proc::MAJOR:       base = juce::Colour (0xffffb347); break; // orange
        case Proc::MINOR:       base = juce::Colour (0xff6699ff); break; // blue
        case Proc::DOM7:        base = juce::Colour (0xffbb77ee); break; // purple
        case Proc::DIM7:        base = juce::Colour (0xffff6666); break; // red
        default:                base = juce::Colours::grey;
    }
    return pressed ? base.brighter (0.5f) : base;
}

//==============================================================================
void StraDellaMIDI_pluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background
    g.fillAll (juce::Colour (0xff1a1a2e));

    const juce::Font labelFont (juce::FontOptions (12.0f, juce::Font::bold));
    const juce::Font noteFont  (juce::FontOptions (11.0f));

    // ── Column headers (note names) ──────────────────────────────────────────
    g.setColour (juce::Colours::lightgrey);
    g.setFont (labelFont);
    for (int col = 0; col < Proc::NUM_COLUMNS; ++col)
    {
        const int x = kLabelW + col * kBtnW;
        g.drawFittedText (Proc::getColumnName (col),
                          x, 0, kBtnW, kHeaderH,
                          juce::Justification::centred, 1);
    }

    // ── Row labels ───────────────────────────────────────────────────────────
    for (int row = 0; row < Proc::NUM_ROWS; ++row)
    {
        const int y = kHeaderH + row * kBtnH;
        g.setColour (rowColour (row, false).withAlpha (0.85f));
        g.fillRect (0, y, kLabelW - 2, kBtnH - 1);

        g.setColour (juce::Colours::black);
        g.setFont (labelFont);
        g.drawFittedText (Proc::getRowName (row),
                          2, y, kLabelW - 4, kBtnH,
                          juce::Justification::centredLeft, 2);
    }

    // ── Button grid ──────────────────────────────────────────────────────────
    for (int row = 0; row < Proc::NUM_ROWS; ++row)
    {
        for (int col = 0; col < Proc::NUM_COLUMNS; ++col)
        {
            const bool pressed = (row == pressedRow && col == pressedCol);
            const auto bounds  = buttonBounds (row, col);

            // Fill
            g.setColour (rowColour (row, pressed));
            g.fillRoundedRectangle (bounds.reduced (2).toFloat(), 5.0f);

            // Border
            g.setColour (pressed ? juce::Colours::white
                                 : juce::Colours::darkgrey);
            g.drawRoundedRectangle (bounds.reduced (2).toFloat(), 5.0f, 1.0f);

            // Note label inside button
            g.setColour (juce::Colours::black);
            g.setFont (noteFont);
            g.drawFittedText (Proc::getColumnName (col),
                              bounds.reduced (3),
                              juce::Justification::centred, 1);
        }
    }
}

void StraDellaMIDI_pluginAudioProcessorEditor::resized()
{
}

//==============================================================================
void StraDellaMIDI_pluginAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    int row, col;
    hitTest (e.getPosition(), row, col);

    if (row >= 0)
    {
        pressedRow = row;
        pressedCol = col;
        audioProcessor.buttonPressed (row, col);
        repaint();
    }
}

void StraDellaMIDI_pluginAudioProcessorEditor::mouseUp (const juce::MouseEvent& /*e*/)
{
    if (pressedRow >= 0)
    {
        audioProcessor.buttonReleased (pressedRow, pressedCol);
        pressedRow = pressedCol = -1;
        repaint();
    }
}
