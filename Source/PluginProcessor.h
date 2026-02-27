/*
  ==============================================================================

    StraDellaMIDI – Stradella Bass Accordion MIDI Effect Plugin
    Emulates the left-hand (Stradella bass) side of an accordion.

    Layout: 12 columns (circle of fifths: Eb→Bb→F→…→Ab) × 4 rows
      Row 0 – Third         (single note: major 3rd above root)
      Row 1 – Bass          (single root note)
      Row 2 – Major row     (major triad; dom7 when left mouse held)
      Row 3 – Minor row     (minor triad; min7 when left mouse held)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/** Per-row voicing parameters exposed to the Expression settings window. */
struct VoicingSettings
{
    // Octave offset per row: -2..+2 (applied as offset * 12 semitones).
    // Index order matches RowType: [COUNTERBASS, BASS, MAJOR, MINOR].
    int octaveOffset[4] = { 0, 0, 0, 0 };

    // Chord inversions for major/minor rows (0 = root, 1 = first, 2 = second).
    int majorInversion = 0;
    int minorInversion = 0;

    // Left mouse button adds the 7th extension.
    bool majorLeftMouseAdds7  = true;
    bool minorLeftMouseAdds7  = true;

    // Right mouse button adds the major 9th.
    bool majorRightMouseAdds9 = true;
    bool minorRightMouseAdds9 = true;
};

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
    // leftMouseDown / rightMouseDown affect chord voicing for major/minor rows.
    void buttonPressed  (int row, int col, int velocity = 100,
                         bool leftMouseDown = false, bool rightMouseDown = false);
    void buttonReleased (int row, int col);

    // Called to queue arbitrary MIDI messages (e.g. CC from mouse expression).
    void addMidiMessage (const juce::MidiMessage& msg);

    // Sends All Notes Off + All Sound Off on all 16 MIDI channels (panic).
    void sendAllNotesOff();

    // Voicing settings accessors.
    void                   setVoicingSettings (const VoicingSettings& s) { voicingSettings = s; }
    const VoicingSettings& getVoicingSettings () const                   { return voicingSettings; }

    // Static helpers – public so the editor can use them for labels.
    juce::Array<int> getNotesForButton (int row, int col,
                                        bool leftMouseDown  = false,
                                        bool rightMouseDown = false) const;
    static int              getRootNote        (int col);
    static juce::String     getColumnName      (int col);
    static juce::String     getRowName         (int row);
    static juce::String     getThirdNoteName   (int col);

private:
    //==============================================================================
    juce::Array<juce::MidiMessage> pendingMessages;
    juce::CriticalSection          messageLock;

    // Tracks which MIDI notes are currently sounding per (row*1000+col) key,
    // so buttonReleased() can send the exact note-offs that were sent on press.
    juce::HashMap<int, juce::Array<int>> activeNotes;

    VoicingSettings voicingSettings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StraDellaMIDI_pluginAudioProcessor)
};
