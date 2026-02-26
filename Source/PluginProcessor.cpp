/*
  ==============================================================================

    StraDellaMIDI – Stradella Bass Accordion MIDI Effect Plugin
    Processor implementation.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Stradella bass layout data
//
// 12 columns arranged in circle-of-fifths order: Bb F C G D A E B F# Db Ab Eb
// Root MIDI notes are in the octave-2 register (MIDI 36 = C2).
//==============================================================================

namespace
{
    // Root MIDI note for each column (circle of fifths, starting on Bb2 = 46).
    static const int kRootNotes[StraDellaMIDI_pluginAudioProcessor::NUM_COLUMNS] = {
        46, // Bb2
        41, // F2
        36, // C2
        43, // G2
        38, // D2
        45, // A2
        40, // E2
        47, // B2
        42, // F#2
        37, // C#2 / Db2
        44, // G#2 / Ab2
        39  // D#2 / Eb2
    };

    static const char* kColumnNames[StraDellaMIDI_pluginAudioProcessor::NUM_COLUMNS] = {
        "Bb", "F", "C", "G", "D", "A", "E", "B", "F#", "Db", "Ab", "Eb"
    };

    static const char* kRowNames[StraDellaMIDI_pluginAudioProcessor::NUM_ROWS] = {
        "Counterbass", "Bass", "Major", "Minor", "Dom 7", "Dim 7"
    };
}

//==============================================================================
int StraDellaMIDI_pluginAudioProcessor::getRootNote (int col)
{
    jassert (col >= 0 && col < NUM_COLUMNS);
    return kRootNotes[col];
}

juce::String StraDellaMIDI_pluginAudioProcessor::getColumnName (int col)
{
    jassert (col >= 0 && col < NUM_COLUMNS);
    return kColumnNames[col];
}

juce::String StraDellaMIDI_pluginAudioProcessor::getRowName (int row)
{
    jassert (row >= 0 && row < NUM_ROWS);
    return kRowNames[row];
}

// Returns the set of MIDI notes sounded when a given button is pressed.
// Chord tones are voiced one octave above the bass root note.
juce::Array<int> StraDellaMIDI_pluginAudioProcessor::getNotesForButton (int row, int col)
{
    const int root = kRootNotes[col];
    const int chordRoot = root + 12; // one octave up for chord voicings

    switch (row)
    {
        case COUNTERBASS:  return { root + 7 };                              // perfect 5th
        case BASS:         return { root };                                  // root only
        case MAJOR:        return { chordRoot, chordRoot + 4, chordRoot + 7 };
        case MINOR:        return { chordRoot, chordRoot + 3, chordRoot + 7 };
        case DOM7:         return { chordRoot, chordRoot + 4, chordRoot + 7, chordRoot + 10 };
        case DIM7:         return { chordRoot, chordRoot + 3, chordRoot + 6, chordRoot + 9 };
        default:           return {};
    }
}

//==============================================================================
StraDellaMIDI_pluginAudioProcessor::StraDellaMIDI_pluginAudioProcessor()
    : AudioProcessor (BusesProperties())   // MIDI effect – no audio buses
{
}

StraDellaMIDI_pluginAudioProcessor::~StraDellaMIDI_pluginAudioProcessor()
{
}

//==============================================================================
const juce::String StraDellaMIDI_pluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

void StraDellaMIDI_pluginAudioProcessor::prepareToPlay (double /*sampleRate*/, int /*samplesPerBlock*/)
{
}

void StraDellaMIDI_pluginAudioProcessor::releaseResources()
{
}

bool StraDellaMIDI_pluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // This is a MIDI-only effect: it must have no audio input or output buses.
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::disabled()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled();
}

void StraDellaMIDI_pluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                       juce::MidiBuffer& midiMessages)
{
    buffer.clear();

    // Drain pending note messages queued by the editor (UI thread).
    juce::Array<juce::MidiMessage> incoming;
    {
        const juce::ScopedLock sl (messageLock);
        incoming.swapWith (pendingMessages);
    }

    for (auto& msg : incoming)
        midiMessages.addEvent (msg, 0);
}

//==============================================================================
bool StraDellaMIDI_pluginAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* StraDellaMIDI_pluginAudioProcessor::createEditor()
{
    return new StraDellaMIDI_pluginAudioProcessorEditor (*this);
}

//==============================================================================
// Called from the UI thread when a stradella button is clicked.
void StraDellaMIDI_pluginAudioProcessor::buttonPressed (int row, int col)
{
    const auto notes = getNotesForButton (row, col);
    const juce::ScopedLock sl (messageLock);
    for (int note : notes)
        pendingMessages.add (juce::MidiMessage::noteOn (1, juce::jlimit (0, 127, note), (juce::uint8) 100));
}

void StraDellaMIDI_pluginAudioProcessor::buttonReleased (int row, int col)
{
    const auto notes = getNotesForButton (row, col);
    const juce::ScopedLock sl (messageLock);
    for (int note : notes)
        pendingMessages.add (juce::MidiMessage::noteOff (1, juce::jlimit (0, 127, note)));
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StraDellaMIDI_pluginAudioProcessor();
}
