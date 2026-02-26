#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Maps computer keyboard keys to MIDI notes based on Stradella accordion layout.
    Supports loading configuration from a text file for flexible key mappings.

    Default keyboard rows (10 of 12 circle-of-fifths pitches: Eb Bb F C G D A E B F#):
      [counterbass]  1 2 3 4 5 6 7 8 9 0   — major 3rd above root  (Stradella row 0)
      [bass]         q w e r t y u i o p   — root note only         (Stradella row 1)
      [major]        a s d f g h j k l ;   — dominant 7th chord     (Stradella row 2)
      [minor]        z x c v b n m , . /   — minor 7th chord        (Stradella row 3)
*/
class StradellaKeyboardMapper
{
public:
    enum class KeyType
    {
        SingleNote,      // Bass row: q,w,e,r,t,y,u,i,o,p — root note only (Stradella bass row)
        ThirdNote,       // Counterbass row: 1 2 3 4 5 6 7 8 9 0 — major 3rd above root
        MajorChord,      // Major row: a,s,d,f,g,h,j,k,l,; — dominant 7th chord
        MinorChord       // Minor row: z,x,c,v,b,n,m,,.,/ — minor 7th chord
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
        rowOut matches PluginProcessor row constants (0=COUNTERBASS … 3=MINOR).
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
