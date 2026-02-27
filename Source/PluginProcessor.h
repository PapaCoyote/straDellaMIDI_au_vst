/*
  ==============================================================================

    StraDellaMIDI – Stradella Bass Accordion MIDI Effect Plugin
    Emulates the left-hand (Stradella bass) side of an accordion.

    Layout: 12 columns (circle of fifths: Eb→Bb→F→…→Ab) × 4 rows
      Row 0 – Third         (single note: major 3rd above root)
      Row 1 – Bass          (single root note)
      Row 2 – Major row     (dominant 7th chord when pressed)
      Row 3 – Minor row     (minor 7th chord when pressed)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class StraDellaMIDI_pluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    static constexpr int NUM_COLUMNS = 12;
    static constexpr int NUM_ROWS    = 4;

    enum RowType { COUNTERBASS = 0, BASS, MAJOR, MINOR };

    //==============================================================================
    StraDellaMIDI_pluginAudioProcessor();
    ~StraDellaMIDI_pluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi()  const override { return true;  }
    bool producesMidi() const override { return true;  }
    bool isMidiEffect() const override { return true;  }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int  getNumPrograms()                                        override { return 1; }
    int  getCurrentProgram()                                     override { return 0; }
    void setCurrentProgram (int)                                 override {}
    const juce::String getProgramName (int)                      override { return {}; }
    void changeProgramName (int, const juce::String&)            override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock&)                override {}
    void setStateInformation (const void*, int)                  override {}

    //==============================================================================
    // Called from the editor (UI thread) to queue note-on / note-off events.
    void buttonPressed  (int row, int col, int velocity = 100);
    void buttonReleased (int row, int col);

    // Called to queue arbitrary MIDI messages (e.g. CC from mouse expression).
    void addMidiMessage (const juce::MidiMessage& msg);

    // Sends All Notes Off + All Sound Off on all 16 MIDI channels (panic).
    void sendAllNotesOff();

    // Static helpers – public so the editor can use them for labels.
    static juce::Array<int> getNotesForButton (int row, int col);
    static int              getRootNote        (int col);
    static juce::String     getColumnName      (int col);
    static juce::String     getRowName         (int row);
    static juce::String     getThirdNoteName   (int col);

private:
    //==============================================================================
    juce::Array<juce::MidiMessage> pendingMessages;
    juce::CriticalSection          messageLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StraDellaMIDI_pluginAudioProcessor)
};
