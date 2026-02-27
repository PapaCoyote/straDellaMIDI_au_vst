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
// Simple content component used inside the three popup windows.
class InfoWindowContent : public juce::Component
{
public:
    explicit InfoWindowContent (const juce::String& title)
    {
        titleLabel.setText (title, juce::dontSendNotification);
        titleLabel.setFont (juce::FontOptions (16.0f, juce::Font::bold));
        titleLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (titleLabel);

        bodyLabel.setText ("Settings coming soon.", juce::dontSendNotification);
        bodyLabel.setFont (juce::FontOptions (12.0f));
        bodyLabel.setJustificationType (juce::Justification::centredTop);
        addAndMakeVisible (bodyLabel);

        setSize (320, 160);
    }

    void resized() override
    {
        titleLabel.setBounds (10, 10, getWidth() - 20, 30);
        bodyLabel .setBounds (10, 50, getWidth() - 20, getHeight() - 60);
    }

private:
    juce::Label titleLabel, bodyLabel;
};

//==============================================================================
StraDellaMIDI_pluginAudioProcessorEditor::StraDellaMIDI_pluginAudioProcessorEditor (
        StraDellaMIDI_pluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Width  = label column + 12 button columns + stagger for last row
    // Height = title + header + 4 button rows + bottom buttons
    const int staggerExtra = (Proc::NUM_ROWS - 1) * kRowOffset;
    const int w = kLabelW + Proc::NUM_COLUMNS * kBtnW + staggerExtra;
    const int h = kTitleH + kHeaderH + Proc::NUM_ROWS * kBtnH + kBottomH;
    setSize (w, h);
    setWantsKeyboardFocus (true);

    // ── Focus toggle button ───────────────────────────────────────────────────
    focusButton.setClickingTogglesState (true);
    focusButton.setToggleState (false, juce::dontSendNotification);
    focusButton.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff3a3a5e));
    focusButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff22aa44));
    focusButton.onClick = [this]
    {
        focusActive = focusButton.getToggleState();
        if (focusActive)
        {
            grabKeyboardFocus();
            aboutButton     .setInterceptsMouseClicks (false, false);
            mappingButton   .setInterceptsMouseClicks (false, false);
            expressionButton.setInterceptsMouseClicks (false, false);

            // Expand window to fill the primary display so mouse events are
            // captured from anywhere on screen.  The area outside the original
            // plugin UI will be rendered at half-transparency.
            originalWidth  = getWidth();
            originalHeight = getHeight();
            setOpaque (false);
            auto* disp = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
            if (disp != nullptr)
                setSize (disp->userArea.getWidth(), disp->userArea.getHeight());
        }
        else
        {
            aboutButton     .setInterceptsMouseClicks (true, true);
            mappingButton   .setInterceptsMouseClicks (true, true);
            expressionButton.setInterceptsMouseClicks (true, true);

            // Restore original plugin size.
            setOpaque (true);
            if (originalWidth > 0 && originalHeight > 0)
                setSize (originalWidth, originalHeight);
            originalWidth = originalHeight = 0;
        }
    };
    addAndMakeVisible (focusButton);

    // ── Panic button ("!") ───────────────────────────────────────────────────
    panicButton.setColour (juce::TextButton::buttonColourId,  juce::Colour (0xffcc2222));
    panicButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    panicButton.onClick = [this]
    {
        // Release any currently held mouse button.
        if (pressedRow >= 0)
        {
            audioProcessor.buttonReleased (pressedRow, pressedCol);
            pressedRow = pressedCol = -1;
        }

        // Release every key held via the computer keyboard.
        for (auto it = activeKeyRow.begin(); it != activeKeyRow.end(); ++it)
        {
            const int row = it.getValue();
            const int col = activeKeyCol[it.getKey()];
            audioProcessor.buttonReleased (row, col);
            keyboardPressedGrid[row][col] = false;
        }
        activeKeyRow.clear();
        activeKeyCol.clear();

        // Broadcast All Notes Off + All Sound Off on all MIDI channels.
        audioProcessor.sendAllNotesOff();
        repaint();
    };
    addAndMakeVisible (panicButton);

    // ── Bottom buttons – each opens a small movable dialog ───────────────────
    auto makeDialog = [this](const juce::String& windowTitle)
    {
        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned (new InfoWindowContent (windowTitle));
        opts.dialogTitle           = windowTitle;
        opts.dialogBackgroundColour= juce::Colour (0xff2a2a3e);
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar     = false;
        opts.resizable             = false;
        opts.launchAsync();
    };

    aboutButton.onClick      = [makeDialog] { makeDialog ("About"); };
    mappingButton.onClick    = [makeDialog] { makeDialog ("Mapping"); };

    // Expression button: show the MouseMidiSettingsWindow inside a dialog.
    expressionButton.onClick = [this]
    {
        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned (new MouseMidiSettingsWindow (mouseExpression, audioProcessor));
        opts.dialogTitle                  = "Expression Settings";
        opts.dialogBackgroundColour       = juce::Colour (0xff2a2a3e);
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar            = false;
        opts.resizable                    = false;
        opts.launchAsync();
    };

    addAndMakeVisible (aboutButton);
    addAndMakeVisible (mappingButton);
    addAndMakeVisible (expressionButton);

    // Wire mouse expression MIDI output to the processor.
    mouseExpression.onMidiMessage = [this] (const juce::MidiMessage& msg)
    {
        audioProcessor.addMidiMessage (msg);
    };

    // When the bellows direction changes, retrigger all held notes.
    mouseExpression.onDirectionChange = [this]
    {
        const int vel = mouseExpression.getCurrentNoteVelocity();
        auto mods = juce::ModifierKeys::getCurrentModifiers();
        const bool leftDown  = mods.isLeftButtonDown();
        const bool rightDown = mods.isRightButtonDown();

        if (pressedRow >= 0)
        {
            audioProcessor.buttonReleased (pressedRow, pressedCol);
            audioProcessor.buttonPressed  (pressedRow, pressedCol, vel, leftDown, rightDown);
        }

        for (auto it = activeKeyRow.begin(); it != activeKeyRow.end(); ++it)
        {
            const int row = it.getValue();
            const int col = activeKeyCol[it.getKey()];
            audioProcessor.buttonReleased (row, col);
            audioProcessor.buttonPressed  (row, col, vel, leftDown, rightDown);
        }
    };

    mouseExpression.startTracking();

    // Register for global focus-change events so Focus mode can re-assert focus.
    juce::Desktop::getInstance().addFocusChangeListener (this);
}

StraDellaMIDI_pluginAudioProcessorEditor::~StraDellaMIDI_pluginAudioProcessorEditor()
{
    juce::Desktop::getInstance().removeFocusChangeListener (this);
}

//==============================================================================
// Returns the pixel bounds for a given button cell.
// Each successive row is shifted kRowOffset pixels to the right.
juce::Rectangle<int>
StraDellaMIDI_pluginAudioProcessorEditor::buttonBounds (int row, int col) const
{
    const int x = kLabelW + col * kBtnW + row * kRowOffset;
    const int y = kTitleH + kHeaderH + row * kBtnH;
    return { x, y, kBtnW, kBtnH };
}

// Converts a pixel position to a row/col index (-1 if outside the grid).
void StraDellaMIDI_pluginAudioProcessorEditor::hitTest (
        juce::Point<int> pos, int& rowOut, int& colOut) const
{
    rowOut = colOut = -1;

    for (int r = 0; r < Proc::NUM_ROWS; ++r)
    {
        const int yTop = kTitleH + kHeaderH + r * kBtnH;
        if (pos.y < yTop || pos.y >= yTop + kBtnH)
            continue;

        const int xOffset = kLabelW + r * kRowOffset;
        if (pos.x < xOffset)
            continue;

        const int c = (pos.x - xOffset) / kBtnW;
        if (c >= 0 && c < Proc::NUM_COLUMNS)
        {
            rowOut = r;
            colOut = c;
        }
        return;
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
        default:                base = juce::Colours::grey;
    }
    return pressed ? base.brighter (0.5f) : base;
}

//==============================================================================
void StraDellaMIDI_pluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // When Focus mode has expanded the window to fill the screen, uiW/uiH define
    // the original plugin UI area.  Everything outside is rendered as a
    // semi-transparent overlay so the underlying desktop is dimly visible.
    const int uiW = (focusActive && originalWidth  > 0) ? originalWidth  : getWidth();
    const int uiH = (focusActive && originalHeight > 0) ? originalHeight : getHeight();

    // Background for the original plugin area (fully opaque dark)
    g.setColour (juce::Colour (0xff1a1a2e));
    g.fillRect (0, 0, uiW, uiH);

    // Semi-transparent dark overlay covering the expanded strips
    if (getWidth() > uiW || getHeight() > uiH)
    {
        g.setColour (juce::Colour (0x80000000));   // black @ 50% alpha
        if (getWidth() > uiW)
            g.fillRect (uiW, 0, getWidth() - uiW, getHeight());    // right strip (full height)
        if (getHeight() > uiH)
            g.fillRect (0, uiH, uiW, getHeight() - uiH);           // bottom strip (left part only)
    }

    // ── Branding / title area ────────────────────────────────────────────────
    {
        // "straDella" in large bold italic (approximates a script font)
        juce::Font titleFont (juce::FontOptions (30.0f, juce::Font::bold | juce::Font::italic));
        g.setColour (juce::Colours::white);
        g.setFont (titleFont);
        g.drawFittedText ("straDella",
                          0, 0, uiW, kTitleH - 18,
                          juce::Justification::centred, 1);

        // "by Papa Coyote" in smaller regular font below
        juce::Font subFont (juce::FontOptions (13.0f));
        g.setColour (juce::Colours::lightgrey);
        g.setFont (subFont);
        g.drawFittedText ("by Papa Coyote",
                          0, kTitleH - 18, uiW, 16,
                          juce::Justification::centred, 1);
    }

    const juce::Font labelFont (juce::FontOptions (12.0f, juce::Font::bold));
    const juce::Font noteFont  (juce::FontOptions (11.0f));

    // ── Column headers (note names, aligned with row 0) ──────────────────────
    g.setColour (juce::Colours::lightgrey);
    g.setFont (labelFont);
    for (int col = 0; col < Proc::NUM_COLUMNS; ++col)
    {
        const int x = kLabelW + col * kBtnW;   // row-0 offset = 0
        g.drawFittedText (Proc::getColumnName (col),
                          x, kTitleH, kBtnW, kHeaderH,
                          juce::Justification::centred, 1);
    }

    // ── Row labels ───────────────────────────────────────────────────────────
    for (int row = 0; row < Proc::NUM_ROWS; ++row)
    {
        const int y = kTitleH + kHeaderH + row * kBtnH;
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
            const bool pressed = (row == pressedRow && col == pressedCol)
                              || keyboardPressedGrid[row][col];
            const auto bounds  = buttonBounds (row, col);

            // Fill
            g.setColour (rowColour (row, pressed));
            g.fillRoundedRectangle (bounds.reduced (2).toFloat(), 5.0f);

            // Border
            g.setColour (pressed ? juce::Colours::white
                                 : juce::Colours::darkgrey);
            g.drawRoundedRectangle (bounds.reduced (2).toFloat(), 5.0f, 1.0f);

            // Note label inside button
            // Third row shows the note a major 3rd above the root;
            // all other rows show the root (column) note name.
            g.setColour (juce::Colours::black);
            g.setFont (noteFont);
            const juce::String label = (row == Proc::COUNTERBASS)
                                       ? Proc::getThirdNoteName (col)
                                       : Proc::getColumnName (col);
            g.drawFittedText (label, bounds.reduced (3),
                              juce::Justification::centred, 1);
        }
    }
}

void StraDellaMIDI_pluginAudioProcessorEditor::resized()
{
    // When in full-screen Focus mode the window may be the size of the entire
    // display.  Always lay out UI elements relative to the original plugin size
    // so they remain in the same position.
    const int uiW = (focusActive && originalWidth > 0) ? originalWidth : getWidth();

    const int btnAreaY = kTitleH + kHeaderH + Proc::NUM_ROWS * kBtnH + 5;
    const int btnH     = kBottomH - 8;
    const int third    = (uiW - 8) / 3;

    // Top-row buttons sit inside the title area.
    static constexpr int kTopBtnY = 10;
    static constexpr int kTopBtnH = 36;
    focusButton.setBounds (5,           kTopBtnY, 100, kTopBtnH);
    panicButton.setBounds (uiW - 65,    kTopBtnY,  60, kTopBtnH);

    aboutButton     .setBounds (2,               btnAreaY, third,     btnH);
    mappingButton   .setBounds (2 + third + 2,   btnAreaY, third,     btnH);
    expressionButton.setBounds (2 + 2 * (third + 2), btnAreaY, third + 2, btnH);
}

//==============================================================================
void StraDellaMIDI_pluginAudioProcessorEditor::globalFocusChanged (juce::Component* focusedComponent)
{
    // When Focus mode is active, re-assert keyboard focus if it moves outside
    // this component's hierarchy (e.g. to the DAW host UI).
    if (!focusActive || focusedComponent == this || isParentOf (focusedComponent))
        return;

    juce::MessageManager::callAsync (
        [safeThis = juce::Component::SafePointer<StraDellaMIDI_pluginAudioProcessorEditor> (this)]
        {
            if (safeThis != nullptr && safeThis->focusActive)
                safeThis->grabKeyboardFocus();
        });
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
        const bool leftDown  = e.mods.isLeftButtonDown();
        const bool rightDown = e.mods.isRightButtonDown();
        audioProcessor.buttonPressed (row, col, mouseExpression.getCurrentNoteVelocity(),
                                      leftDown, rightDown);
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

//==============================================================================
bool StraDellaMIDI_pluginAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    const int keyCode = key.getKeyCode();

    // Ignore auto-repeated key events (key already active).
    if (activeKeyRow.contains (keyCode))
        return true;

    int row, col;
    if (keyboardMapper.getButtonCoords (keyCode, row, col))
    {
        activeKeyRow.set (keyCode, row);
        activeKeyCol.set (keyCode, col);
        keyboardPressedGrid[row][col] = true;
        auto mods = juce::ModifierKeys::getCurrentModifiers();
        audioProcessor.buttonPressed (row, col, mouseExpression.getCurrentNoteVelocity(),
                                      mods.isLeftButtonDown(), mods.isRightButtonDown());
        repaint();
        return true;
    }
    return false;
}

bool StraDellaMIDI_pluginAudioProcessorEditor::keyStateChanged (bool isKeyDown)
{
    if (isKeyDown)
        return false;   // key-down events are handled by keyPressed()

    // A key was released — find any active keyboard buttons that are no longer held.
    juce::Array<int> toRelease;
    for (auto it = activeKeyRow.begin(); it != activeKeyRow.end(); ++it)
        if (!juce::KeyPress::isKeyCurrentlyDown (it.getKey()))
            toRelease.add (it.getKey());

    for (int keyCode : toRelease)
    {
        const int row = activeKeyRow[keyCode];
        const int col = activeKeyCol[keyCode];
        audioProcessor.buttonReleased (row, col);
        keyboardPressedGrid[row][col] = false;
        activeKeyRow.remove (keyCode);
        activeKeyCol.remove (keyCode);
    }

    if (!toRelease.isEmpty())
    {
        repaint();
        return true;
    }
    return false;
}
