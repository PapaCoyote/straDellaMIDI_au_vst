#include "StradellaKeyboardMapper.h"

//==============================================================================
StradellaKeyboardMapper::StradellaKeyboardMapper()
{
    loadDefaultConfiguration();
}

void StradellaKeyboardMapper::loadDefaultConfiguration()
{
    setupDefaultMappings();
}

void StradellaKeyboardMapper::setupDefaultMappings()
{
    keyMappings.clear();
    
    // Row 1: Single notes in cycle of fifths (a,s,d,f,g,h,j,k,l,;)
    // All notes in Octave 1 (MIDI 24-35) as per Stradella bass system
    // F key = C1 (MIDI note 24)
    // Cycle of fifths: each step is +7 semitones (or -5 going backwards)
    // Since we're limited to octave 1, we wrap around within the octave
    
    // Mapping for single note row (a,s,d,f,g,h,j,k,l,;) - removed apostrophe
    juce::Array<int> singleNoteKeys = { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';' };
    
    // Cycle of fifths within octave 1 (all notes MIDI 24-35)
    // A, S, D, F, G, H, J, K, L, ;
    // Cycle of fifths pattern: Eb, Bb, F, C, G, D, A, E, B, F#
    juce::Array<int> singleNoteMidiValues = { 27, 34, 29, 24, 31, 26, 33, 28, 35, 30 }; // Octave 1 notes
    
    for (int i = 0; i < singleNoteKeys.size(); ++i)
    {
        KeyMapping mapping;
        mapping.keyCode = singleNoteKeys[i];
        mapping.type = KeyType::SingleNote;
        mapping.midiNotes.add(singleNoteMidiValues[i]);
        mapping.description = getMidiNoteName(singleNoteMidiValues[i]);
        keyMappings.set(mapping.keyCode, mapping);
    }
    
    // Row 2: Third above (z,x,c,v,b,n,m,comma,period,slash)
    // These are a major third (4 semitones) above the corresponding single notes
    // Also keeping within octave 1
    juce::Array<int> thirdNoteKeys = { 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/' };
    
    // Third notes - major third above, wrapping to stay in octave 1
    juce::Array<int> thirdNoteMidiValues = { 31, 26, 33, 28, 35, 30, 25, 32, 27, 34 }; // Octave 1 notes
    
    for (int i = 0; i < thirdNoteKeys.size(); ++i)
    {
        KeyMapping mapping;
        mapping.keyCode = thirdNoteKeys[i];
        mapping.type = KeyType::ThirdNote;
        mapping.midiNotes.add(thirdNoteMidiValues[i]);
        mapping.description = getMidiNoteName(thirdNoteMidiValues[i]);
        keyMappings.set(mapping.keyCode, mapping);
    }
    
    // Row 3: Major triads (q,w,e,r,t,y,u,i,o,p)
    // Major triad: root, major third (+4), perfect fifth (+7)
    // All notes in Octave 2 (MIDI 36-47) as per Stradella bass system
    // Cycle of fifths starting from Eb: Eb, Bb, F, C, G, D, A, E, B, F#
    juce::Array<int> majorChordKeys = { 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P' };
    juce::Array<int> majorChordRoots = { 39, 46, 41, 36, 43, 38, 45, 40, 47, 42 }; // Eb2, Bb2, F2, C2, G2, D2, A2, E2, B2, F#2
    
    for (int i = 0; i < majorChordKeys.size() && i < majorChordRoots.size(); ++i)
    {
        KeyMapping mapping;
        mapping.keyCode = majorChordKeys[i];
        mapping.type = KeyType::MajorChord;
        int root = majorChordRoots[i];
        mapping.midiNotes.add(root);        // Root
        mapping.midiNotes.add(root + 4);    // Major third
        mapping.midiNotes.add(root + 7);    // Perfect fifth
        mapping.description = getMidiNoteName(root) + " Major";
        keyMappings.set(mapping.keyCode, mapping);
    }
    
    // Row 4: Minor triads (1,2,3,4,5,6,7,8,9,0)
    // Minor triad: root, minor third (+3), perfect fifth (+7)
    // All notes in Octave 2 (MIDI 36-47) as per Stradella bass system
    // Cycle of fifths starting from Eb: Eb, Bb, F, C, G, D, A, E, B, F#
    juce::Array<int> minorChordKeys = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    juce::Array<int> minorChordRoots = { 39, 46, 41, 36, 43, 38, 45, 40, 47, 42 }; // Eb2, Bb2, F2, C2, G2, D2, A2, E2, B2, F#2
    
    for (int i = 0; i < minorChordKeys.size() && i < minorChordRoots.size(); ++i)
    {
        KeyMapping mapping;
        mapping.keyCode = minorChordKeys[i];
        mapping.type = KeyType::MinorChord;
        int root = minorChordRoots[i];
        mapping.midiNotes.add(root);        // Root
        mapping.midiNotes.add(root + 3);    // Minor third
        mapping.midiNotes.add(root + 7);    // Perfect fifth
        mapping.description = getMidiNoteName(root) + " Minor";
        keyMappings.set(mapping.keyCode, mapping);
    }
}

juce::Array<int> StradellaKeyboardMapper::getMidiNotesForKey(int keyCode, bool& isValidKey) const
{
    if (keyMappings.contains(keyCode))
    {
        isValidKey = true;
        return keyMappings[keyCode].midiNotes;
    }
    
    isValidKey = false;
    return {};
}

StradellaKeyboardMapper::KeyType StradellaKeyboardMapper::getKeyType(int keyCode) const
{
    if (keyMappings.contains(keyCode))
        return keyMappings[keyCode].type;
    
    return KeyType::SingleNote; // Default
}

juce::String StradellaKeyboardMapper::getKeyDescription(int keyCode) const
{
    if (keyMappings.contains(keyCode))
        return keyMappings[keyCode].description;
    
    return {};
}

juce::String StradellaKeyboardMapper::getMidiNoteName(int midiNoteNumber)
{
    static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    
    int octave = (midiNoteNumber / 12) - 1;
    int noteIndex = midiNoteNumber % 12;
    
    return juce::String(noteNames[noteIndex]) + juce::String(octave);
}

bool StradellaKeyboardMapper::loadConfiguration(const juce::File& configFile)
{
    if (!configFile.existsAsFile())
        return false;
    
    // TODO: Implement configuration file parsing
    // For now, just use default configuration
    loadDefaultConfiguration();
    return true;
}
