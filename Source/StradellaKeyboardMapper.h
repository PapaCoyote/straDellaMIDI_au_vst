#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Maps computer keyboard keys to MIDI notes based on Stradella accordion layout.
    Supports loading configuration from a text file for flexible key mappings.

    Default keyboard rows (10 of 12 circle-of-fifths pitches: Eb Bb F C G D A E B F#):
      [counterbass]  z x c v b n m , . /   — perfect 5th above root (Stradella row 0)
      [bass]         a s d f g h j k l ;   — root note only         (Stradella row 1)
      [major]        q w e r t y u i o p   — major triad            (Stradella row 2)
      [minor]        1 2 3 4 5 6 7 8 9 0   — minor triad            (Stradella row 3)
    [dom7] and [dim7] rows are not mapped by default but are available via config file.
*/
class StradellaKeyboardMapper
{
public:
    enum class KeyType
    {
        SingleNote,      // Bass row: a,s,d,f,g,h,j,k,l,; — root note only (Stradella bass row)
        ThirdNote,       // Counterbass row: z x c v b n m , . / — perfect 5th above root
        MajorChord,      // Major chord row: q,w,e,r,t,y,u,i,o,p — root, major 3rd, perfect 5th
        MinorChord,      // Minor chord row: 1,2,3,4,5,6,7,8,9,0 — root, minor 3rd, perfect 5th
        Dom7Chord,       // Dominant 7th chord — root, major 3rd, perfect 5th, minor 7th
        Dim7Chord        // Diminished 7th chord — root, minor 3rd, diminished 5th, diminished 7th
    };

    //==============================================================================
    StradellaKeyboardMapper();
    
    /** Loads keyboard mappings from a configuration file */
    bool loadConfiguration(const juce::File& configFile);
    
    /** Loads default keyboard mappings */
    void loadDefaultConfiguration();
    
    /** Gets MIDI notes for a given key press */
    juce::Array<int> getMidiNotesForKey(int keyCode, bool& isValidKey) const;
    
    /** Gets the key type for a given key code */
    KeyType getKeyType(int keyCode) const;
    
    /** Gets a human-readable name for a MIDI note number */
    static juce::String getMidiNoteName(int midiNoteNumber);
    
    /** Gets a human-readable description for a key */
    juce::String getKeyDescription(int keyCode) const;

    /**
        Maps a keyboard key code to its corresponding plugin grid coordinates.
        Returns true and sets rowOut/colOut when a mapping is found.
        rowOut matches PluginProcessor row constants (0=COUNTERBASS … 5=DIM7).
        colOut matches the circle-of-fifths column (0=Bb … 11=Eb).
    */
    bool getButtonCoords(int keyCode, int& rowOut, int& colOut) const;

private:
    struct KeyMapping
    {
        int keyCode;
        KeyType type;
        juce::Array<int> midiNotes;
        juce::String description;
        int pluginRow { -1 };   ///< Corresponding row in the plugin's Stradella grid
        int pluginCol { -1 };   ///< Corresponding column in the plugin's circle-of-fifths grid
    };
    
    juce::HashMap<int, KeyMapping> keyMappings;
    
    void setupDefaultMappings();

    /** Normalises a key code so letters are always lowercase for consistent lookup. */
    static int normalizeKeyCode(int keyCode);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StradellaKeyboardMapper)
};
