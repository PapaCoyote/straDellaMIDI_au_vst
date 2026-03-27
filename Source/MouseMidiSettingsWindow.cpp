#include "MouseMidiSettingsWindow.h"

//==============================================================================
MouseMidiSettingsWindow::MouseMidiSettingsWindow (MouseMidiExpression& midiExpression,
                                                  StraDellaMIDI_pluginAudioProcessor& processor)
    : mouseMidiExpression (midiExpression), audioProcessor (processor)
{
    setupUI();
    setSize (440, 240);
}

MouseMidiSettingsWindow::~MouseMidiSettingsWindow() {}

//==============================================================================
void MouseMidiSettingsWindow::setupUI()
{
    // ── Title ─────────────────────────────────────────────────────────────────
    titleLabel.setText ("Expression Settings", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (17.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    // ── Section header ────────────────────────────────────────────────────────
    expressionSectionLabel.setText ("Mouse Expression", juce::dontSendNotification);
    expressionSectionLabel.setFont (juce::Font (12.0f, juce::Font::bold));
    expressionSectionLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (expressionSectionLabel);

    // ── CC1 (Modulation Wheel) ─────────────────────────────────────────────────
    modulationLabel.setText ("CC1 (Modulation) — Y position while moving:", juce::dontSendNotification);
    addAndMakeVisible (modulationLabel);
    modulationCheckbox.setToggleState (mouseMidiExpression.isModulationEnabled(), juce::dontSendNotification);
    modulationCheckbox.onClick = [this] { mouseMidiExpression.setModulationEnabled (modulationCheckbox.getToggleState()); };
    addAndMakeVisible (modulationCheckbox);

    // ── CC11 (Expression) ─────────────────────────────────────────────────────
    expressionLabel.setText ("CC11 (Expression) — Y position while moving:", juce::dontSendNotification);
    addAndMakeVisible (expressionLabel);
    expressionCheckbox.setToggleState (mouseMidiExpression.isExpressionEnabled(), juce::dontSendNotification);
    expressionCheckbox.onClick = [this] { mouseMidiExpression.setExpressionEnabled (expressionCheckbox.getToggleState()); };
    addAndMakeVisible (expressionCheckbox);

    // ── Retrigger ─────────────────────────────────────────────────────────────
    retriggerLabel.setText ("Retrigger notes on bellows direction change:", juce::dontSendNotification);
    addAndMakeVisible (retriggerLabel);
    retriggerCheckbox.setToggleState (mouseMidiExpression.isRetriggerOnDirectionChangeEnabled(), juce::dontSendNotification);
    retriggerCheckbox.onClick = [this] { mouseMidiExpression.setRetriggerOnDirectionChange (retriggerCheckbox.getToggleState()); };
    addAndMakeVisible (retriggerCheckbox);

    // ── Curve selector ────────────────────────────────────────────────────────
    curveLabel.setText ("Response Curve:", juce::dontSendNotification);
    addAndMakeVisible (curveLabel);
    curveSelector.addItem ("Linear",      1);
    curveSelector.addItem ("Exponential", 2);
    curveSelector.addItem ("Logarithmic", 3);
    switch (mouseMidiExpression.getCurveType())
    {
        case MouseMidiExpression::CurveType::Exponential: curveSelector.setSelectedId (2, juce::dontSendNotification); break;
        case MouseMidiExpression::CurveType::Logarithmic: curveSelector.setSelectedId (3, juce::dontSendNotification); break;
        default:                                          curveSelector.setSelectedId (1, juce::dontSendNotification); break;
    }
    curveSelector.onChange = [this]
    {
        switch (curveSelector.getSelectedId())
        {
            case 2:  mouseMidiExpression.setCurveType (MouseMidiExpression::CurveType::Exponential); break;
            case 3:  mouseMidiExpression.setCurveType (MouseMidiExpression::CurveType::Logarithmic); break;
            default: mouseMidiExpression.setCurveType (MouseMidiExpression::CurveType::Linear);      break;
        }
    };
    addAndMakeVisible (curveSelector);

    // ── Close button ──────────────────────────────────────────────────────────
    closeButton.setButtonText ("Close");
    closeButton.onClick = [this]
    {
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->closeButtonPressed();
        else
            setVisible (false);
    };
    addAndMakeVisible (closeButton);
}

//==============================================================================
void MouseMidiSettingsWindow::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 2);
}

void MouseMidiSettingsWindow::resized()
{
    const int m  = 15;   // outer margin
    const int rh = 22;   // standard row height
    const int sh = 16;   // section header height
    const int g  =  6;   // gap between rows

    auto area = getLocalBounds().reduced (m);

    // ── Title ─────────────────────────────────────────────────────────────────
    titleLabel.setBounds (area.removeFromTop (28));
    area.removeFromTop (8);

    // ── Mouse Expression section ──────────────────────────────────────────────
    expressionSectionLabel.setBounds (area.removeFromTop (sh));
    area.removeFromTop (4);

    auto makeCheckRow = [&](juce::ToggleButton& cb, juce::Label& lbl)
    {
        auto row = area.removeFromTop (rh);
        cb.setBounds  (row.removeFromLeft (22));
        lbl.setBounds (row);
        area.removeFromTop (g);
    };
    makeCheckRow (modulationCheckbox, modulationLabel);
    makeCheckRow (expressionCheckbox, expressionLabel);
    makeCheckRow (retriggerCheckbox,  retriggerLabel);

    {
        auto row = area.removeFromTop (rh);
        curveLabel.setBounds    (row.removeFromLeft (120));
        curveSelector.setBounds (row.reduced (2, 0));
        area.removeFromTop (g);
    }

    // ── Close button ──────────────────────────────────────────────────────────
    area.removeFromTop (12);
    closeButton.setBounds (area.removeFromTop (30).withSizeKeepingCentre (100, 28));
}
