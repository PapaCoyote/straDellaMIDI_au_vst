#include "MappingSettingsWindow.h"

//==============================================================================
// Octave ComboBox: IDs 1-5 → offsets -2,-1, 0,+1,+2  (ID 3 = "Default")
void MappingSettingsWindow::populateOctaveBox (juce::ComboBox& box, int currentOffset)
{
    box.addItem ("-2 Octaves", 1);
    box.addItem ("-1 Octave",  2);
    box.addItem ("Default",    3);
    box.addItem ("+1 Octave",  4);
    box.addItem ("+2 Octaves", 5);
    box.setSelectedId (offsetToOctaveBoxId (currentOffset), juce::dontSendNotification);
}

void MappingSettingsWindow::populateInversionBox (juce::ComboBox& box, int currentInversion)
{
    box.addItem ("Root Position", 1);
    box.addItem ("1st Inversion", 2);
    box.addItem ("2nd Inversion", 3);
    box.setSelectedId (currentInversion + 1, juce::dontSendNotification);
}

int MappingSettingsWindow::octaveBoxToOffset (int id)     { return juce::jlimit (-2, 2, id - 3); }
int MappingSettingsWindow::offsetToOctaveBoxId (int offset) { return juce::jlimit (1, 5, offset + 3); }

//==============================================================================
MappingSettingsWindow::MappingSettingsWindow (StraDellaMIDI_pluginAudioProcessor& processor)
    : audioProcessor (processor)
{
    setupUI();
    setSize (440, 420);
}

MappingSettingsWindow::~MappingSettingsWindow() {}

//==============================================================================
void MappingSettingsWindow::setupUI()
{
    const auto& vs = audioProcessor.getVoicingSettings();

    // ── Title ─────────────────────────────────────────────────────────────────
    titleLabel.setText ("Voicing Settings", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (17.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    // ── Section header ────────────────────────────────────────────────────────
    auto makeSectionHeader = [this](juce::Label& lbl, const juce::String& text, juce::Colour colour)
    {
        lbl.setText (text, juce::dontSendNotification);
        lbl.setFont (juce::Font (12.0f, juce::Font::bold));
        lbl.setColour (juce::Label::textColourId, colour);
        addAndMakeVisible (lbl);
    };
    makeSectionHeader (voicingSectionLabel, "Voicing",    juce::Colours::lightgrey);
    makeSectionHeader (majorRowLabel,       "Major Row",  juce::Colour (0xffffb347));
    makeSectionHeader (minorRowLabel,       "Minor Row",  juce::Colour (0xff6699ff));

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
void MappingSettingsWindow::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 2);
}

void MappingSettingsWindow::resized()
{
    const int m  = 15;   // outer margin
    const int rh = 22;   // standard row height
    const int sh = 16;   // section header height
    const int g  =  6;   // gap between rows

    auto area = getLocalBounds().reduced (m);

    // ── Title ─────────────────────────────────────────────────────────────────
    titleLabel.setBounds (area.removeFromTop (28));
    area.removeFromTop (8);

    // ── Voicing section header ────────────────────────────────────────────────
    voicingSectionLabel.setBounds (area.removeFromTop (sh));
    area.removeFromTop (4);

    auto makeOctRow = [&](juce::Label& lbl, juce::ComboBox& box)
    {
        auto row = area.removeFromTop (rh);
        lbl.setBounds (row.removeFromLeft (130));
        box.setBounds (row.reduced (2, 0));
        area.removeFromTop (g);
    };
    makeOctRow (thirdOctLabel, thirdOctaveBox);
    makeOctRow (bassOctLabel,  bassOctaveBox);

    area.removeFromTop (8);

    auto makeCheckRow = [&](juce::ToggleButton& cb, juce::Label& lbl)
    {
        auto row = area.removeFromTop (rh);
        cb.setBounds  (row.removeFromLeft (22));
        lbl.setBounds (row);
        area.removeFromTop (g);
    };

    // ── Major Row ─────────────────────────────────────────────────────────────
    majorRowLabel.setBounds (area.removeFromTop (sh));
    area.removeFromTop (4);

    {
        auto row = area.removeFromTop (rh);
        const int half = row.getWidth() / 2;
        auto left  = row.removeFromLeft (half);
        auto right = row;
        majorOctLabel.setBounds      (left.removeFromLeft (60));
        majorOctaveBox.setBounds     (left.reduced (2, 0));
        majorInvLabel.setBounds      (right.removeFromLeft (65));
        majorInversionBox.setBounds  (right.reduced (2, 0));
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
        minorOctLabel.setBounds      (left.removeFromLeft (60));
        minorOctaveBox.setBounds     (left.reduced (2, 0));
        minorInvLabel.setBounds      (right.removeFromLeft (65));
        minorInversionBox.setBounds  (right.reduced (2, 0));
        area.removeFromTop (g);
    }
    makeCheckRow (minorLmbToggle, minorLmbLabel);
    makeCheckRow (minorRmbToggle, minorRmbLabel);

    // ── Close button ──────────────────────────────────────────────────────────
    area.removeFromTop (12);
    closeButton.setBounds (area.removeFromTop (30).withSizeKeepingCentre (100, 28));
}
