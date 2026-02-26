#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Maps computer keyboard keys to MIDI notes based on Stradella accordion layout.
    Supports loading configuration from a text file for flexible key mappings.
*/
class StradellaKeyboardMapper
{
public:
    enum class KeyType
    {
        SingleNote,      // Row: a,s,d,f,g,h,j,k (cycle of fifths)
        ThirdNote,       // Row: z,x,c,v,b,n,m (third above)
        MajorChord,      // Row: q,w,e,r,t,y,u,i,o,p
        MinorChord       // Row: 1,2,3,4,5,6,7
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

private:
    struct KeyMapping
    {
        int keyCode;
        KeyType type;
        juce::Array<int> midiNotes;
        juce::String description;
    };
    
    juce::HashMap<int, KeyMapping> keyMappings;
    
    void setupDefaultMappings();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StradellaKeyboardMapper)
};
