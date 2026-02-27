#include "MouseMidiSettingsWindow.h"

//==============================================================================
// Octave ComboBox: IDs 1-5 → offsets -2,-1, 0,+1,+2  (ID 3 = "Default")
void MouseMidiSettingsWindow::populateOctaveBox (juce::ComboBox& box, int currentOffset)
{
    box.addItem ("-2 Octaves", 1);
    box.addItem ("-1 Octave",  2);
    box.addItem ("Default",    3);
    box.addItem ("+1 Octave",  4);
    box.addItem ("+2 Octaves", 5);
    box.setSelectedId (offsetToOctaveBoxId (currentOffset), juce::dontSendNotification);
}

void MouseMidiSettingsWindow::populateInversionBox (juce::ComboBox& box, int currentInversion)
{
    box.addItem ("Root Position", 1);
    box.addItem ("1st Inversion", 2);
    box.addItem ("2nd Inversion", 3);
    box.setSelectedId (currentInversion + 1, juce::dontSendNotification);
}

int MouseMidiSettingsWindow::octaveBoxToOffset (int id) { return juce::jlimit (-2, 2, id - 3); }
int MouseMidiSettingsWindow::offsetToOctaveBoxId (int offset) { return juce::jlimit (1, 5, offset + 3); }

//==============================================================================
MouseMidiSettingsWindow::MouseMidiSettingsWindow (MouseMidiExpression& midiExpression,
                                                  StraDellaMIDI_pluginAudioProcessor& processor)
    : mouseMidiExpression (midiExpression), audioProcessor (processor)
{
    setupUI();
    setSize (440, 560);
}

MouseMidiSettingsWindow::~MouseMidiSettingsWindow() {}

//==============================================================================
void MouseMidiSettingsWindow::setupUI()
{
    const auto& vs = audioProcessor.getVoicingSettings();

    // ── Title ─────────────────────────────────────────────────────────────────
    titleLabel.setText ("Expression & Voicing Settings", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (17.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    // ── Section headers ───────────────────────────────────────────────────────
    auto makeSectionHeader = [this](juce::Label& lbl, const juce::String& text, juce::Colour colour)
    {
        lbl.setText (text, juce::dontSendNotification);
        lbl.setFont (juce::Font (12.0f, juce::Font::bold));
        lbl.setColour (juce::Label::textColourId, colour);
        addAndMakeVisible (lbl);
    };
    makeSectionHeader (expressionSectionLabel, "Mouse Expression", juce::Colours::lightgrey);
    makeSectionHeader (voicingSectionLabel,    "Voicing",          juce::Colours::lightgrey);
    makeSectionHeader (majorRowLabel,          "Major Row",        juce::Colour (0xffffb347));
    makeSectionHeader (minorRowLabel,          "Minor Row",        juce::Colour (0xff6699ff));

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

    // ── Third row octave ──────────────────────────────────────────────────────
    thirdOctLabel.setText ("Third row octave:", juce::dontSendNotification);
    addAndMakeVisible (thirdOctLabel);
    populateOctaveBox (thirdOctaveBox, vs.octaveOffset[StraDellaMIDI_pluginAudioProcessor::COUNTERBASS]);
    thirdOctaveBox.onChange = [this]
    {
        auto s = audioProcessor.getVoicingSettings();
        s.octaveOffset[StraDellaMIDI_pluginAudioProcessor::COUNTERBASS] = octaveBoxToOffset (thirdOctaveBox.getSelectedId());
        audioProcessor.setVoicingSettings (s);
    };
    addAndMakeVisible (thirdOctaveBox);

    // ── Bass row octave ───────────────────────────────────────────────────────
    bassOctLabel.setText ("Bass row octave:", juce::dontSendNotification);
    addAndMakeVisible (bassOctLabel);
    populateOctaveBox (bassOctaveBox, vs.octaveOffset[StraDellaMIDI_pluginAudioProcessor::BASS]);
    bassOctaveBox.onChange = [this]
    {
        auto s = audioProcessor.getVoicingSettings();
        s.octaveOffset[StraDellaMIDI_pluginAudioProcessor::BASS] = octaveBoxToOffset (bassOctaveBox.getSelectedId());
        audioProcessor.setVoicingSettings (s);
    };
    addAndMakeVisible (bassOctaveBox);

    // ── Major row ─────────────────────────────────────────────────────────────
    majorOctLabel.setText ("Octave:", juce::dontSendNotification);
    addAndMakeVisible (majorOctLabel);
    populateOctaveBox (majorOctaveBox, vs.octaveOffset[StraDellaMIDI_pluginAudioProcessor::MAJOR]);
    majorOctaveBox.onChange = [this]
    {
        auto s = audioProcessor.getVoicingSettings();
        s.octaveOffset[StraDellaMIDI_pluginAudioProcessor::MAJOR] = octaveBoxToOffset (majorOctaveBox.getSelectedId());
        audioProcessor.setVoicingSettings (s);
    };
    addAndMakeVisible (majorOctaveBox);

    majorInvLabel.setText ("Inversion:", juce::dontSendNotification);
    addAndMakeVisible (majorInvLabel);
    populateInversionBox (majorInversionBox, vs.majorInversion);
    majorInversionBox.onChange = [this]
    {
        auto s = audioProcessor.getVoicingSettings();
        s.majorInversion = juce::jlimit (0, 2, majorInversionBox.getSelectedId() - 1);
        audioProcessor.setVoicingSettings (s);
    };
    addAndMakeVisible (majorInversionBox);

    majorLmbLabel.setText ("Left mouse adds dominant 7th:", juce::dontSendNotification);
    addAndMakeVisible (majorLmbLabel);
    majorLmbToggle.setToggleState (vs.majorLeftMouseAdds7, juce::dontSendNotification);
    majorLmbToggle.onClick = [this]
    {
        auto s = audioProcessor.getVoicingSettings();
        s.majorLeftMouseAdds7 = majorLmbToggle.getToggleState();
        audioProcessor.setVoicingSettings (s);
    };
    addAndMakeVisible (majorLmbToggle);

    majorRmbLabel.setText ("Right mouse adds major 9th:", juce::dontSendNotification);
    addAndMakeVisible (majorRmbLabel);
    majorRmbToggle.setToggleState (vs.majorRightMouseAdds9, juce::dontSendNotification);
    majorRmbToggle.onClick = [this]
    {
        auto s = audioProcessor.getVoicingSettings();
        s.majorRightMouseAdds9 = majorRmbToggle.getToggleState();
        audioProcessor.setVoicingSettings (s);
    };
    addAndMakeVisible (majorRmbToggle);

    // ── Minor row ─────────────────────────────────────────────────────────────
    minorOctLabel.setText ("Octave:", juce::dontSendNotification);
    addAndMakeVisible (minorOctLabel);
    populateOctaveBox (minorOctaveBox, vs.octaveOffset[StraDellaMIDI_pluginAudioProcessor::MINOR]);
    minorOctaveBox.onChange = [this]
    {
        auto s = audioProcessor.getVoicingSettings();
        s.octaveOffset[StraDellaMIDI_pluginAudioProcessor::MINOR] = octaveBoxToOffset (minorOctaveBox.getSelectedId());
        audioProcessor.setVoicingSettings (s);
    };
    addAndMakeVisible (minorOctaveBox);

    minorInvLabel.setText ("Inversion:", juce::dontSendNotification);
    addAndMakeVisible (minorInvLabel);
    populateInversionBox (minorInversionBox, vs.minorInversion);
    minorInversionBox.onChange = [this]
    {
        auto s = audioProcessor.getVoicingSettings();
        s.minorInversion = juce::jlimit (0, 2, minorInversionBox.getSelectedId() - 1);
        audioProcessor.setVoicingSettings (s);
    };
    addAndMakeVisible (minorInversionBox);

    minorLmbLabel.setText ("Left mouse adds minor 7th:", juce::dontSendNotification);
    addAndMakeVisible (minorLmbLabel);
    minorLmbToggle.setToggleState (vs.minorLeftMouseAdds7, juce::dontSendNotification);
    minorLmbToggle.onClick = [this]
    {
        auto s = audioProcessor.getVoicingSettings();
        s.minorLeftMouseAdds7 = minorLmbToggle.getToggleState();
        audioProcessor.setVoicingSettings (s);
    };
    addAndMakeVisible (minorLmbToggle);

    minorRmbLabel.setText ("Right mouse adds major 9th:", juce::dontSendNotification);
    addAndMakeVisible (minorRmbLabel);
    minorRmbToggle.setToggleState (vs.minorRightMouseAdds9, juce::dontSendNotification);
    minorRmbToggle.onClick = [this]
    {
        auto s = audioProcessor.getVoicingSettings();
        s.minorRightMouseAdds9 = minorRmbToggle.getToggleState();
        audioProcessor.setVoicingSettings (s);
    };
    addAndMakeVisible (minorRmbToggle);

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

    area.removeFromTop (10);

    // ── Voicing section ────────────────────────────────────────────────────────
    voicingSectionLabel.setBounds (area.removeFromTop (sh));
    area.removeFromTop (4);

    auto makeOctRow = [&](juce::Label& lbl, juce::ComboBox& box)
    {
        auto row = area.removeFromTop (rh);
        lbl.setBounds  (row.removeFromLeft (130));
        box.setBounds  (row.reduced (2, 0));
        area.removeFromTop (g);
    };
    makeOctRow (thirdOctLabel, thirdOctaveBox);
    makeOctRow (bassOctLabel,  bassOctaveBox);

    area.removeFromTop (8);

    // ── Major Row ─────────────────────────────────────────────────────────────
    majorRowLabel.setBounds (area.removeFromTop (sh));
    area.removeFromTop (4);

    {
        // Octave + Inversion on one row (split 50/50 after the first label)
        auto row = area.removeFromTop (rh);
        const int half = row.getWidth() / 2;
        auto left  = row.removeFromLeft (half);
        auto right = row;
        majorOctLabel.setBounds    (left.removeFromLeft (60));
        majorOctaveBox.setBounds   (left.reduced (2, 0));
        majorInvLabel.setBounds    (right.removeFromLeft (65));
        majorInversionBox.setBounds (right.reduced (2, 0));
        area.removeFromTop (g);
    }
    makeCheckRow (majorLmbToggle, majorLmbLabel);
    makeCheckRow (majorRmbToggle, majorRmbLabel);

    area.removeFromTop (8);

    // ── Minor Row ─────────────────────────────────────────────────────────────
    minorRowLabel.setBounds (area.removeFromTop (sh));
    area.removeFromTop (4);

    {
        auto row = area.removeFromTop (rh);
        const int half = row.getWidth() / 2;
        auto left  = row.removeFromLeft (half);
        auto right = row;
        minorOctLabel.setBounds    (left.removeFromLeft (60));
        minorOctaveBox.setBounds   (left.reduced (2, 0));
        minorInvLabel.setBounds    (right.removeFromLeft (65));
        minorInversionBox.setBounds (right.reduced (2, 0));
        area.removeFromTop (g);
    }
    makeCheckRow (minorLmbToggle, minorLmbLabel);
    makeCheckRow (minorRmbToggle, minorRmbLabel);

    // ── Close button ──────────────────────────────────────────────────────────
    area.removeFromTop (12);
    closeButton.setBounds (area.removeFromTop (30).withSizeKeepingCentre (100, 28));
}
