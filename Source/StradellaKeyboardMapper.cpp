#include "StradellaKeyboardMapper.h"

//==============================================================================
// Root MIDI notes for each of the 12 circle-of-fifths columns, matching
// PluginProcessor::kRootNotes exactly (all in octave 2, MIDI 36-47):
//   Col:  0   1   2   3   4   5   6   7   8   9  10  11
//   Note: Bb  F   C   G   D   A   E   B   F#  Db  Ab  Eb
namespace
{
    static const int kColumnRoots[12] = {
        46, // Bb2
        41, // F2
        36, // C2
        43, // G2
        38, // D2
        45, // A2
        40, // E2
        47, // B2
        42, // F#2 / Gb2
        37, // C#2 / Db2
        44, // G#2 / Ab2
        39  // D#2 / Eb2
    };

    // Maps KeyType to its plugin grid row index.
    static int keyTypeToPluginRow (StradellaKeyboardMapper::KeyType t)
    {
        switch (t)
        {
            case StradellaKeyboardMapper::KeyType::ThirdNote:  return 0; // COUNTERBASS
            case StradellaKeyboardMapper::KeyType::SingleNote: return 1; // BASS
            case StradellaKeyboardMapper::KeyType::MajorChord: return 2; // MAJOR
            case StradellaKeyboardMapper::KeyType::MinorChord: return 3; // MINOR
            case StradellaKeyboardMapper::KeyType::Dom7Chord:  return 4; // DOM7
            case StradellaKeyboardMapper::KeyType::Dim7Chord:  return 5; // DIM7
            default: return -1;
        }
    }
}

//==============================================================================
StradellaKeyboardMapper::StradellaKeyboardMapper()
{
    loadDefaultConfiguration();
}

void StradellaKeyboardMapper::loadDefaultConfiguration()
{
    setupDefaultMappings();
}

int StradellaKeyboardMapper::normalizeKeyCode (int keyCode)
{
    if (keyCode >= 'A' && keyCode <= 'Z')
        return keyCode + ('a' - 'A');
    return keyCode;
}

void StradellaKeyboardMapper::setupDefaultMappings()
{
    keyMappings.clear();

    // The 10 mapped keys cover the most-used circle-of-fifths positions:
    //   Keyboard: a  s  d  f  g  h  j  k  l  ;
    //   Pitch:    Eb Bb F  C  G  D  A  E  B  F#
    //   Col:      11  0  1  2  3  4  5  6  7  8
    static const int kNumKeys = 10;
    static const int kKeyCols[kNumKeys]  = { 11, 0, 1, 2, 3, 4, 5, 6, 7, 8 };
    static const int kBassKeys[kNumKeys] = { 'a','s','d','f','g','h','j','k','l',';' };
    static const int kCBKeys  [kNumKeys] = { 'z','x','c','v','b','n','m',',','.','/' };
    static const int kMajKeys [kNumKeys] = { 'q','w','e','r','t','y','u','i','o','p' };
    static const int kMinKeys [kNumKeys] = { '1','2','3','4','5','6','7','8','9','0' };

    for (int i = 0; i < kNumKeys; ++i)
    {
        const int col       = kKeyCols[i];
        const int root      = kColumnRoots[col];  // bass note, octave 2
        const int cbNote    = root + 7;            // perfect 5th (counterbass)
        const int chordRoot = root + 12;           // chord root, octave 3

        const juce::String rootName = getMidiNoteName (root);

        // ── Row 1: Bass (single root note) ───────────────────────────────────
        {
            KeyMapping m;
            m.keyCode   = kBassKeys[i];
            m.type      = KeyType::SingleNote;
            m.midiNotes.add (root);
            m.description = rootName + " Bass";
            m.pluginRow = 1;
            m.pluginCol = col;
            keyMappings.set (m.keyCode, m);
        }

        // ── Row 0: Counterbass (perfect 5th above root) ──────────────────────
        {
            KeyMapping m;
            m.keyCode   = kCBKeys[i];
            m.type      = KeyType::ThirdNote;
            m.midiNotes.add (cbNote);
            m.description = rootName + " Counterbass";
            m.pluginRow = 0;
            m.pluginCol = col;
            keyMappings.set (m.keyCode, m);
        }

        // ── Row 2: Major triad (chordRoot, M3, P5) ───────────────────────────
        {
            KeyMapping m;
            m.keyCode   = kMajKeys[i];
            m.type      = KeyType::MajorChord;
            m.midiNotes.add (chordRoot);
            m.midiNotes.add (chordRoot + 4);   // major 3rd
            m.midiNotes.add (chordRoot + 7);   // perfect 5th
            m.description = rootName + " Major";
            m.pluginRow = 2;
            m.pluginCol = col;
            keyMappings.set (m.keyCode, m);
        }

        // ── Row 3: Minor triad (chordRoot, m3, P5) ───────────────────────────
        {
            KeyMapping m;
            m.keyCode   = kMinKeys[i];
            m.type      = KeyType::MinorChord;
            m.midiNotes.add (chordRoot);
            m.midiNotes.add (chordRoot + 3);   // minor 3rd
            m.midiNotes.add (chordRoot + 7);   // perfect 5th
            m.description = rootName + " Minor";
            m.pluginRow = 3;
            m.pluginCol = col;
            keyMappings.set (m.keyCode, m);
        }
    }
}

//==============================================================================
juce::Array<int> StradellaKeyboardMapper::getMidiNotesForKey (int keyCode, bool& isValidKey) const
{
    const int norm = normalizeKeyCode (keyCode);
    if (keyMappings.contains (norm))
    {
        isValidKey = true;
        return keyMappings[norm].midiNotes;
    }
    isValidKey = false;
    return {};
}

StradellaKeyboardMapper::KeyType StradellaKeyboardMapper::getKeyType (int keyCode) const
{
    const int norm = normalizeKeyCode (keyCode);
    if (keyMappings.contains (norm))
        return keyMappings[norm].type;
    return KeyType::SingleNote;
}

juce::String StradellaKeyboardMapper::getKeyDescription (int keyCode) const
{
    const int norm = normalizeKeyCode (keyCode);
    if (keyMappings.contains (norm))
        return keyMappings[norm].description;
    return {};
}

bool StradellaKeyboardMapper::getButtonCoords (int keyCode, int& rowOut, int& colOut) const
{
    const int norm = normalizeKeyCode (keyCode);
    if (keyMappings.contains (norm))
    {
        rowOut = keyMappings[norm].pluginRow;
        colOut = keyMappings[norm].pluginCol;
        return (rowOut >= 0 && colOut >= 0);
    }
    rowOut = colOut = -1;
    return false;
}

juce::String StradellaKeyboardMapper::getMidiNoteName (int midiNoteNumber)
{
    static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    const int octave    = (midiNoteNumber / 12) - 1;
    const int noteIndex = midiNoteNumber % 12;
    return juce::String (noteNames[noteIndex]) + juce::String (octave);
}

//==============================================================================
bool StradellaKeyboardMapper::loadConfiguration (const juce::File& configFile)
{
    if (!configFile.existsAsFile())
        return false;

    // Start from the default mappings and override with file contents.
    setupDefaultMappings();

    juce::StringArray lines;
    configFile.readLines (lines);

    // Current section determines the KeyType for subsequent key=value lines.
    // Supported: [bass], [counterbass], [major], [minor], [dom7], [dim7]
    KeyType currentSection = KeyType::SingleNote;

    for (const auto& rawLine : lines)
    {
        const auto line = rawLine.trim();
        if (line.isEmpty() || line.startsWith ("#"))
            continue;

        // Section header, e.g. "[major]"
        if (line.startsWith ("["))
        {
            const auto name = line.substring (1, line.indexOf ("]")).trim().toLowerCase();
            if      (name == "bass")        currentSection = KeyType::SingleNote;
            else if (name == "counterbass") currentSection = KeyType::ThirdNote;
            else if (name == "major")       currentSection = KeyType::MajorChord;
            else if (name == "minor")       currentSection = KeyType::MinorChord;
            else if (name == "dom7")        currentSection = KeyType::Dom7Chord;
            else if (name == "dim7")        currentSection = KeyType::Dim7Chord;
            continue;
        }

        // key = note1[,note2,...] [# comment]
        const int eqPos = line.indexOf ("=");
        if (eqPos <= 0)
            continue;

        const auto keyStr = line.substring (0, eqPos).trim();
        if (keyStr.isEmpty())
            continue;
        const int keyCode = normalizeKeyCode (static_cast<int> (static_cast<juce::juce_wchar> (keyStr[0])));

        // Strip trailing comment, then parse comma-separated note values.
        auto valStr = line.substring (eqPos + 1);
        const int hashPos = valStr.indexOf ("#");
        if (hashPos >= 0)
            valStr = valStr.substring (0, hashPos);
        valStr = valStr.trim();

        juce::Array<int> notes;
        for (const auto& tok : juce::StringArray::fromTokens (valStr, ",", ""))
        {
            const int n = tok.trim().getIntValue();
            if (n >= 0 && n <= 127)
                notes.add (n);
        }

        if (notes.isEmpty())
            continue;

        // Determine the root note so we can find the plugin column.
        //   SingleNote  → notes[0] is the root (octave 2)
        //   ThirdNote   → notes[0] is root+7;  root = notes[0]-7
        //   Chord types → notes[0] is root+12; root = notes[0]-12
        int rootNote = notes[0];
        switch (currentSection)
        {
            case KeyType::ThirdNote:  rootNote = notes[0] - 7;  break;
            case KeyType::MajorChord:
            case KeyType::MinorChord:
            case KeyType::Dom7Chord:
            case KeyType::Dim7Chord:  rootNote = notes[0] - 12; break;
            default: break;
        }

        int col = -1;
        for (int c = 0; c < 12; ++c)
            if (kColumnRoots[c] == rootNote) { col = c; break; }

        KeyMapping mapping;
        mapping.keyCode   = keyCode;
        mapping.type      = currentSection;
        mapping.midiNotes = notes;
        mapping.pluginRow = keyTypeToPluginRow (currentSection);
        mapping.pluginCol = col;
        mapping.description = getMidiNoteName (notes.size() == 1 ? notes[0] : rootNote);
        keyMappings.set (keyCode, mapping);
    }

    return true;
}
