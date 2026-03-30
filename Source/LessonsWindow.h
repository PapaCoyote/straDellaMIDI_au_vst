/*
  ==============================================================================

    StraDellaMIDI – Stradella Bass Accordion MIDI Effect Plugin
    Lessons window declaration.

    Provides an interactive lesson system for learning the Stradella bass.
    Each lesson contains topics with explanatory text followed by a scored
    exercise: a sequence of stradella buttons the learner must play in order.

    Lesson markup files use a simple line-based format:

        # comment
        LESSON: Title of the lesson
        DESCRIPTION: Short description shown in the selector drop-down

        TOPIC: Topic title
        TEXT: A line of explanatory text
        TEXT: Another line
        SCORE: rftf-rftf|tgyg-tgyg

    where SCORE is a string of keyboard key characters (same keys used to play the
    accordion buttons), with:
        - (dash)  inserting a visible gap between note groups
        | (pipe)  starting a new score line

    The keyboard layout is:
        Counterbass: 1 2 3 4 5 6 7 8 9 0   (cols 0-9: Eb Bb F C G D A E B F#)
        Bass:        q w e r t y u i o p
        Major:       a s d f g h j k l ;
        Minor:       z x c v b n m , . /

    A legacy colon format is also accepted for backward compatibility:
        SCORE: bass:C major:G minor:A bass:F
    where rowname : third | bass | major | minor
          colname : Eb Bb F C G D A E B F# Db Ab

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/** A single note (or layout sentinel) in a scored exercise.
    Real notes have row 0–3 and col 0–11.
    Sentinels carry layout information only and are not played:
      row == -2  →  group-space (rendered as a visual gap between note groups)
      row == -3  →  line-break  (score continues on a new display line)
*/
struct LessonScoreNote
{
    int row { -1 };
    int col { -1 };

    bool operator== (const LessonScoreNote& o) const noexcept
    {
        return row == o.row && col == o.col;
    }

    /** True for a playable note (row 0–3). */
    bool isNote      () const noexcept { return row >= 0; }
    /** True for a group-space sentinel (dash in SCORE markup). */
    bool isSpace     () const noexcept { return row == -2; }
    /** True for a line-break sentinel (pipe in SCORE markup). */
    bool isLineBreak () const noexcept { return row == -3; }
};

//==============================================================================
/** One topic block: title, explanatory text lines, and a playable scored sequence. */
struct LessonTopic
{
    juce::String                 title;
    juce::StringArray            textLines;
    juce::Array<LessonScoreNote> score;
};

//==============================================================================
/** Full lesson data loaded from a .lesson markup file or embedded string. */
struct LessonData
{
    juce::String             title;
    juce::String             description;
    juce::Array<LessonTopic> topics;
};

//==============================================================================
class LessonPanel;   // inner scrollable component defined in LessonsWindow.cpp

//==============================================================================
/**
    Non-modal lessons window shown alongside the main plugin UI.

    Features:
      - A drop-down to choose among loaded lessons.
      - A scrollable panel of topic text + scored exercises.
      - Triangle play button per score that plays back the notes.
      - Keyboard input handling: user presses the indicated keys in sequence;
        correct answers flash the score and advance to the next topic, wrong
        answers show an unobtrusive "Try again" message.
*/
class LessonsWindow : public juce::Component,
                      private juce::Timer
{
public:
    explicit LessonsWindow (StraDellaMIDI_pluginAudioProcessor& processor);
    ~LessonsWindow() override;

    //==========================================================================
    void paint   (juce::Graphics&) override;
    void resized () override;

    bool keyPressed      (const juce::KeyPress&) override;
    bool keyStateChanged (bool isKeyDown)        override;

    /** Returns the default lesson markup bundled with the plugin. */
    static juce::String getBuiltInLessonsText();

private:
    //==========================================================================
    void timerCallback() override;

    void loadLessons();
    void showLesson  (int index);
    void refreshPanel();
    void scrollToCurrentTopic();

    void startPlayback (int topicIdx);
    void stopPlayback  ();
    void onPlayTick    ();
    void onFlashTick   ();

    void handleNotePress   (int row, int col);
    void handleNoteRelease (int row, int col);
    void onScoreComplete   ();
    void advanceTopic      ();

    static juce::Array<LessonData> parseLessons (const juce::String& text);

    //==========================================================================
    StraDellaMIDI_pluginAudioProcessor& audioProcessor;

    juce::Label      selectorLabel;
    juce::ComboBox   lessonSelector;
    juce::TextButton textSmallerButton { "A-" };   ///< shrink lesson text font
    juce::TextButton textLargerButton  { "A+" };   ///< grow   lesson text font
    // panel must be declared before viewport so that it is destroyed AFTER viewport
    // (members are destroyed in reverse declaration order).
    std::unique_ptr<LessonPanel> panel;
    juce::Viewport   viewport;
    juce::TextButton closeButton { "Close" };

    juce::Array<LessonData> lessons;
    int  currentLessonIdx { -1 };

    // Lesson text font size — adjustable via A+/A- buttons (range 10–20, default 13)
    float textFontSize { 13.0f };

    // User score-matching progress
    int  userTopicIdx { 0 };
    int  userNoteIdx  { 0 };
    bool tryAgain     { false };

    // Timer state
    enum class TimerMode { Idle, Playback, Flash };
    TimerMode timerMode { TimerMode::Idle };

    int  playbackTopicIdx  { -1 };
    int  playbackNoteIdx   {  0 };
    bool playbackPhase     { false };   ///< false = note-on half, true = note-off half
    int  highlightedNoteIdx{ -1 };      ///< which score note is visually highlighted

    // Flash animation after a correct answer
    bool flashOn    { false };
    int  flashCount {  0 };

    // Active keyboard presses: keyCode → row / col
    juce::HashMap<int,int> activeKeyRow;
    juce::HashMap<int,int> activeKeyCol;

    static constexpr int kMargin       = 10;
    static constexpr int kComboH       = 26;
    static constexpr int kCloseBtnH    = 28;
    static constexpr int kPlayTimerMs  = 250;   ///< half-beat at 120 BPM (note-on or note-off)
    static constexpr int kFlashTimerMs = 150;   ///< flash-toggle interval

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LessonsWindow)
};
