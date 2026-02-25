/*
  ==============================================================================

    StraDellaMIDI – Stradella Bass Accordion MIDI Effect Plugin
    Emulates the left-hand (Stradella bass) side of an accordion.

    Layout: 12 columns (circle of fifths: Bb→F→C→…→Eb) × 6 rows
      Row 0 – Counterbass  (single note: root + perfect 5th)
      Row 1 – Bass         (single root note)
      Row 2 – Major chord
      Row 3 – Minor chord
      Row 4 – Dominant 7th chord
      Row 5 – Diminished 7th chord

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
    static constexpr int NUM_ROWS    = 6;

    enum RowType { COUNTERBASS = 0, BASS, MAJOR, MINOR, DOM7, DIM7 };

    //==============================================================================
    StraDellaMIDI_pluginAudioProcessor();
    ~StraDellaMIDI_pluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
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
    void buttonPressed  (int row, int col);
    void buttonReleased (int row, int col);

    // Static helpers – public so the editor can use them for labels.
    static juce::Array<int> getNotesForButton (int row, int col);
    static int              getRootNote        (int col);
    static juce::String     getColumnName      (int col);
    static juce::String     getRowName         (int row);

private:
    //==============================================================================
    juce::Array<juce::MidiMessage> pendingMessages;
    juce::CriticalSection          messageLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StraDellaMIDI_pluginAudioProcessor)
};
