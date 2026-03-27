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
// 12 columns arranged in circle-of-fifths order: Eb Bb F C G D A E B F# Db Ab
// Eb is placed first to align with the leftmost keyboard key ('q').
// Root MIDI notes are in the octave-2 register (MIDI 36 = C2).
//==============================================================================

namespace
{
    // Root MIDI note for each column (Eb-first circle-of-fifths order).
    static const int kRootNotes[StraDellaMIDI_pluginAudioProcessor::NUM_COLUMNS] = {
        39, // D#2 / Eb2
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
        44  // G#2 / Ab2
    };

    static const char* kColumnNames[StraDellaMIDI_pluginAudioProcessor::NUM_COLUMNS] = {
        "Eb", "Bb", "F", "C", "G", "D", "A", "E", "B", "F#", "Db", "Ab"
    };

    // Name of the major-3rd note above each column root (for the Third row).
    static const char* kThirdNames[StraDellaMIDI_pluginAudioProcessor::NUM_COLUMNS] = {
        "G",  // Eb + M3
        "D",  // Bb + M3
        "A",  // F  + M3
        "E",  // C  + M3
        "B",  // G  + M3
        "F#", // D  + M3
        "C#", // A  + M3
        "G#", // E  + M3
        "Eb", // B  + M3 (D# enharmonic)
        "Bb", // F# + M3 (A# enharmonic)
        "F",  // Db + M3
        "C"   // Ab + M3
    };

    static const char* kRowNames[StraDellaMIDI_pluginAudioProcessor::NUM_ROWS] = {
        "Third", "Bass", "Major", "Minor"
    };

    // Move the lowest note up an octave for each inversion step.
    // Notes array must be in ascending order on entry (always true for our chords:
    // we build them as [base, base+3/4, base+7, base+10, base+14] in that order).
    // After adding notes[0]+12 (always > notes.back() for standard chord intervals
    // of ≤14 semitones) and removing notes[0], the array stays sorted.
    static void applyInversion (juce::Array<int>& notes, int inversion)
    {
        jassert (inversion >= 0 && inversion <= 2);
        for (int i = 0; i < inversion; ++i)
        {
            if (notes.size() < 2) break;
            // notes[0] + 12 always lands above the existing highest note because
            // the widest interval in our chords (root → 9th) is only 14 semitones.
            notes.add (notes[0] + 12);
            notes.remove (0);
        }
    }
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

juce::String StraDellaMIDI_pluginAudioProcessor::getThirdNoteName (int col)
{
    jassert (col >= 0 && col < NUM_COLUMNS);
    return kThirdNames[col];
}

// Returns the set of MIDI notes sounded when a given button is pressed.
// Chord tones are voiced one octave above the bass root note (plus any octave offset).
// For the major and minor rows the chord type is extended when mouse buttons are held:
//   left mouse down  → adds the 7th (dominant 7 / minor 7) when the setting is enabled
//   right mouse down → adds the major 9th when the setting is enabled
juce::Array<int> StraDellaMIDI_pluginAudioProcessor::getNotesForButton (
        int row, int col, bool leftMouseDown, bool rightMouseDown) const
{
    jassert (col >= 0 && col < NUM_COLUMNS);
    jassert (row >= 0 && row < NUM_ROWS);

    const int root      = kRootNotes[col];
    const int octShift  = voicingSettings.octaveOffset[row] * 12;

    switch (row)
    {
        case COUNTERBASS:
            return { root + 4 + octShift };

        case BASS:
            return { root + octShift };

        case MAJOR:
        {
            const int base       = root + 12 + octShift;
            const bool addSev    = leftMouseDown  && voicingSettings.majorLeftMouseAdds7;
            const bool addNinth  = rightMouseDown && voicingSettings.majorRightMouseAdds9;

            juce::Array<int> notes;
            notes.add (base);
            notes.add (base + 4);   // major 3rd
            notes.add (base + 7);   // perfect 5th
            if (addSev)   notes.add (base + 10);  // minor 7th  → dominant 7
            if (addNinth) notes.add (base + 14);  // major 9th

            applyInversion (notes, voicingSettings.majorInversion);
            return notes;
        }

        case MINOR:
        {
            const int base       = root + 12 + octShift;
            const bool addSev    = leftMouseDown  && voicingSettings.minorLeftMouseAdds7;
            const bool addNinth  = rightMouseDown && voicingSettings.minorRightMouseAdds9;

            juce::Array<int> notes;
            notes.add (base);
            notes.add (base + 3);   // minor 3rd
            notes.add (base + 7);   // perfect 5th
            if (addSev)   notes.add (base + 10);  // minor 7th
            if (addNinth) notes.add (base + 14);  // major 9th

            applyInversion (notes, voicingSettings.minorInversion);
            return notes;
        }

        default:
            return {};
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
void StraDellaMIDI_pluginAudioProcessor::buttonPressed (int row, int col, int velocity,
                                                         bool leftMouseDown, bool rightMouseDown)
{
    const int key = row * 1000 + col;
    const juce::ScopedLock sl (messageLock);

    // Increment reference count.  Send note-on only the first time the cell
    // is pressed (count rises from 0 → 1); subsequent presses from a second
    // input source (mouse + keyboard simultaneously) are ignored so that a
    // single note-off from either source cannot leave a stuck note.
    const int count = pressCount.contains (key) ? pressCount[key] : 0;
    pressCount.set (key, count + 1);

    if (count == 0)
    {
        const auto notes = getNotesForButton (row, col, leftMouseDown, rightMouseDown);
        activeNotes.set (key, notes);
        for (int note : notes)
            pendingMessages.add (juce::MidiMessage::noteOn (1, juce::jlimit (0, 127, note),
                                                            (juce::uint8) juce::jlimit (0, 127, velocity)));
    }
}

void StraDellaMIDI_pluginAudioProcessor::buttonReleased (int row, int col)
{
    const int key = row * 1000 + col;
    const juce::ScopedLock sl (messageLock);

    if (!pressCount.contains (key))
        return;

    const int newCount = pressCount[key] - 1;
    if (newCount > 0)
    {
        // Another source is still holding the cell; just decrement.
        pressCount.set (key, newCount);
        return;
    }

    // Count reached 0: send note-offs and clean up.
    pressCount.remove (key);
    if (activeNotes.contains (key))
    {
        for (int note : activeNotes[key])
            pendingMessages.add (juce::MidiMessage::noteOff (1, juce::jlimit (0, 127, note)));
        activeNotes.remove (key);
    }
}

void StraDellaMIDI_pluginAudioProcessor::addMidiMessage (const juce::MidiMessage& msg)
{
    const juce::ScopedLock sl (messageLock);
    pendingMessages.add (msg);
}

void StraDellaMIDI_pluginAudioProcessor::sendAllNotesOff()
{
    const juce::ScopedLock sl (messageLock);
    activeNotes.clear();
    pressCount.clear();
    for (int ch = 1; ch <= 16; ++ch)
    {
        pendingMessages.add (juce::MidiMessage::allNotesOff (ch));
        pendingMessages.add (juce::MidiMessage::allSoundOff (ch));
    }
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StraDellaMIDI_pluginAudioProcessor();
}
