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
        SCORE: bass:C major:G minor:A bass:F

    where SCORE tokens are  rowname:colname  pairs, e.g.
        rowname : third | bass | major | minor
        colname : Eb | Bb | F | C | G | D | A | E | B | F# | Db | Ab

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/** A single note in a scored exercise (row 0-3, col 0-11 of the Stradella grid). */
struct LessonScoreNote
{
    int row { -1 };
    int col { -1 };

    bool operator== (const LessonScoreNote& o) const noexcept
    {
        return row == o.row && col == o.col;
    }
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
    static juce::String            getKeyChar   (int row, int col);

    //==========================================================================
    StraDellaMIDI_pluginAudioProcessor& audioProcessor;

    juce::Label      selectorLabel;
    juce::ComboBox   lessonSelector;
    // panel must be declared before viewport so that it is destroyed AFTER viewport
    // (members are destroyed in reverse declaration order).
    std::unique_ptr<LessonPanel> panel;
    juce::Viewport   viewport;
    juce::TextButton closeButton { "Close" };

    juce::Array<LessonData> lessons;
    int  currentLessonIdx { -1 };

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
