/*
  ==============================================================================

    StraDellaMIDI – Stradella Bass Accordion MIDI Effect Plugin
    Lessons window implementation.

  ==============================================================================
*/

#include "LessonsWindow.h"
#include "StradellaKeyboardMapper.h"

using Proc = StraDellaMIDI_pluginAudioProcessor;

//==============================================================================
// Score helpers – shared by LessonPanel (display) and LessonsWindow (logic).
//==============================================================================

/** Counts only playable notes in a score, ignoring layout sentinels. */
static int countNotes (const juce::Array<LessonScoreNote>& score) noexcept
{
    int n = 0;
    for (const auto& note : score)
        if (note.isNote()) ++n;
    return n;
}

/**
    Returns the flat-array index of the nth playable note (0-based), or -1 if
    nth is out of range.  Sentinels do not advance the count.
*/
static int findNthNote (const juce::Array<LessonScoreNote>& score, int nth) noexcept
{
    int count = 0;
    for (int si = 0; si < score.size(); ++si)
    {
        if (score[si].isNote())
        {
            if (count == nth) return si;
            ++count;
        }
    }
    return -1;
}

/** Returns the number of display lines in a score (1 + number of line-break sentinels). */
static int countScoreLines (const juce::Array<LessonScoreNote>& score) noexcept
{
    int lines = 1;
    for (const auto& n : score)
        if (n.isLineBreak()) ++lines;
    return lines;
}

//==============================================================================
// LessonPanel – inner scrollable component for rendering lesson content.
// Owned by LessonsWindow and placed inside its juce::Viewport.
//==============================================================================
class LessonPanel : public juce::Component
{
public:
    //==========================================================================
    /** Rendering state pushed from LessonsWindow every time something changes. */
    struct State
    {
        int  playbackTopicIdx  { -1 };   ///< topic whose score is being played (-1 = none)
        int  highlightedNoteIdx{ -1 };   ///< which score note is currently highlighted
        int  userTopicIdx      {  0 };   ///< topic the user is currently expected to play
        int  userNoteIdx       {  0 };   ///< how many notes of the score the user has matched
        bool tryAgain          { false };
        bool flashOn           { false };
    };

    //==========================================================================
    LessonPanel()
    {
        setWantsKeyboardFocus (false);   // key events are handled by LessonsWindow
    }

    void setLesson (const LessonData& d)
    {
        lesson = d;
        recalcLayouts();
        repaint();
    }

    void setState (const State& s)
    {
        state = s;
        recalcLayouts();   // try-again message can change topic height
        repaint();
    }

    /** Sets the lesson text font size and reflows layout. */
    void setTextFontSize (float sizePt)
    {
        textFontSize = sizePt;
        recalcLayouts();
        repaint();
    }

    int getPreferredHeight() const noexcept { return preferredHeight; }

    int getTopicY (int topicIdx) const noexcept
    {
        if (topicIdx >= 0 && topicIdx < (int) layouts.size())
            return layouts[topicIdx].y;
        return 0;
    }

    /** Hit-test the play button for each topic. Returns true and sets topicIdxOut. */
    bool hitTestPlayButton (juce::Point<int> pos, int& topicIdxOut) const
    {
        for (int i = 0; i < lesson.topics.size(); ++i)
        {
            if (lesson.topics[i].score.isEmpty()) continue;
            const int by = layouts[i].scoreY + (kScoreAreaH - kPlayBtnSize) / 2;
            const juce::Rectangle<int> btnR (kMargin, by, kPlayBtnSize, kPlayBtnSize);
            if (btnR.contains (pos))
            {
                topicIdxOut = i;
                return true;
            }
        }
        return false;
    }

    /** Called when the user clicks a play button. */
    std::function<void(int)> onPlayButtonClicked;

    //==========================================================================
    void mouseDown (const juce::MouseEvent& e) override
    {
        int topicIdx = -1;
        if (hitTestPlayButton (e.getPosition(), topicIdx) && onPlayButtonClicked)
            onPlayButtonClicked (topicIdx);
    }

    //==========================================================================
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff1e1e2e));

        // Title is 2pt larger than the text font; both scale with textFontSize.
        const juce::Font titleFont (juce::FontOptions (textFontSize + 2.0f, juce::Font::bold));
        const juce::Font textFont  (juce::FontOptions (textFontSize));
        const juce::Font noteFont  (juce::FontOptions (14.0f, juce::Font::bold));  // score note-name
        const juce::Font smallFont (juce::FontOptions (9.0f));
        const juce::Font keyFont   (juce::FontOptions (17.0f, juce::Font::bold));  // score key label

        // Compute the same dynamic heights used in recalcLayouts().
        const int titleH    = juce::jmax (kTopicTitleH, (int)(textFontSize + 16.0f));
        const int textLineH = juce::jmax (16, (int)(textFontSize + 7.0f));

        for (int ti = 0; ti < lesson.topics.size(); ++ti)
        {
            const auto& topic  = lesson.topics.getReference (ti);
            const auto& layout = layouts[ti];

            // ── Separator line between topics ─────────────────────────────────
            if (ti > 0)
            {
                g.setColour (juce::Colour (0xff404060));
                g.drawHorizontalLine (layout.y - kTopicGap / 2,
                                      (float) kMargin, (float)(getWidth() - kMargin));
            }

            // ── Topic title ───────────────────────────────────────────────────
            g.setColour (juce::Colours::white);
            g.setFont (titleFont);
            g.drawText (topic.title,
                        kMargin, layout.y, getWidth() - 2 * kMargin, titleH,
                        juce::Justification::centredLeft);

            // ── Text lines ────────────────────────────────────────────────────
            g.setColour (juce::Colour (0xffccccdd));
            g.setFont (textFont);
            for (int li = 0; li < topic.textLines.size(); ++li)
            {
                g.drawText (topic.textLines[li],
                            kMargin + 4,
                            layout.y + titleH + li * textLineH,
                            getWidth() - 2 * kMargin - 8, textLineH,
                            juce::Justification::centredLeft);
            }

            // ── Score area ────────────────────────────────────────────────────
            if (!topic.score.isEmpty())
            {
                const int  scoreY      = layout.scoreY;
                const bool isThisTopic = (state.playbackTopicIdx == ti);

                // ── Play / pause button (triangle) ────────────────────────────
                const int  by    = scoreY + (kScoreAreaH - kPlayBtnSize) / 2;
                const auto btnR  = juce::Rectangle<float> ((float) kMargin, (float) by,
                                                            (float) kPlayBtnSize,
                                                            (float) kPlayBtnSize);

                g.setColour (isThisTopic ? juce::Colour (0xff55ff55)
                                         : juce::Colour (0xff33aa33));
                juce::Path tri;
                tri.addTriangle (btnR.getX(),     btnR.getY(),
                                 btnR.getX(),     btnR.getBottom(),
                                 btnR.getRight(), btnR.getCentreY());
                g.fillPath (tri);

                // ── Score notes ───────────────────────────────────────────────
                const bool isFlashing = (state.flashOn
                                         && state.userTopicIdx == ti
                                         && state.userNoteIdx >= countNotes (topic.score));

                int nx = kMargin + kPlayBtnSize + kScoreGap;
                int lineIdx = 0;   // current display line (incremented by line-break sentinels)
                int ni = 0;        // actual note index (sentinels excluded)

                for (const auto& sn : topic.score)
                {
                    // ── Layout sentinels ──────────────────────────────────────
                    if (sn.isLineBreak())
                    {
                        nx = kMargin + kPlayBtnSize + kScoreGap;
                        ++lineIdx;
                        continue;
                    }
                    if (sn.isSpace())
                    {
                        nx += kScoreSpaceW;
                        continue;
                    }

                    // ── Playable note ─────────────────────────────────────────
                    const int ny   = scoreY + lineIdx * kScoreAreaH
                                     + (kScoreAreaH - kScoreNoteH) / 2;

                    const bool isHighlighted = isThisTopic && (state.highlightedNoteIdx == ni);
                    const bool isPlayed      = (ti < state.userTopicIdx)
                                           || (ti == state.userTopicIdx && ni < state.userNoteIdx);

                    // Show note/chord name when highlighted, already played, or flashing;
                    // otherwise show the keyboard key character.
                    const bool showNoteName  = isHighlighted || isPlayed || isFlashing;

                    const int expand = isHighlighted ? 5 : 0;
                    const juce::Rectangle<int> noteBounds (
                        nx - expand / 2, ny - expand / 2,
                        kScoreNoteW + expand, kScoreNoteH + expand);

                    juce::Colour nc = rowColour (sn.row);
                    if (isFlashing)       nc = nc.brighter (0.8f);
                    else if (isPlayed)    nc = nc.withAlpha (0.45f);

                    g.setColour (nc);
                    g.fillRoundedRectangle (noteBounds.toFloat(), 4.0f);

                    g.setColour (isHighlighted ? juce::Colours::white
                                               : juce::Colour (0xff505050));
                    g.drawRoundedRectangle (noteBounds.toFloat(), 4.0f, 1.0f);

                    // Label: keyboard key by default; note name when highlighted/played/flashing
                    g.setColour (juce::Colours::black);
                    const juce::String label = showNoteName
                                             ? Proc::getColumnName (sn.col)
                                             : StradellaKeyboardMapper::getKeyLabel (sn.row, sn.col);
                    g.setFont (showNoteName ? noteFont : keyFont);
                    g.drawFittedText (label,
                                      noteBounds.withTrimmedBottom (showNoteName ? 10 : 0)
                                               .reduced (2),
                                      juce::Justification::centred, 1);

                    // Row-type abbreviation at bottom — only shown with the note name
                    if (showNoteName)
                    {
                        g.setFont (smallFont);
                        g.setColour (juce::Colours::black.withAlpha (0.55f));
                        g.drawFittedText (rowAbbrev (sn.row),
                                          juce::Rectangle<int> (nx, ny + kScoreNoteH - 11,
                                                                 kScoreNoteW, 11),
                                          juce::Justification::centred, 1);
                    }

                    nx += kScoreNoteW + kScoreNoteGap;
                    ++ni;
                }

                // ── "Try again" message ───────────────────────────────────────
                if (state.tryAgain && state.userTopicIdx == ti)
                {
                    const int numLines = countScoreLines (topic.score);
                    g.setColour (juce::Colour (0xffff7777));
                    g.setFont (textFont);
                    g.drawText ("Try again",
                                kMargin + kPlayBtnSize + kScoreGap,
                                scoreY + numLines * kScoreAreaH + 2,
                                120, 18,
                                juce::Justification::centredLeft);
                }
            }
        }
    }

    void resized() override { recalcLayouts(); }

private:
    //==========================================================================
    LessonData lesson;
    State      state;
    float      textFontSize { 13.0f };   ///< lesson text font size in points (adjustable)

    struct TopicLayout
    {
        int y       { 0 };   ///< top of the topic block
        int scoreY  { 0 };   ///< top of the score area within the topic
        int height  { 0 };   ///< total height of the topic block
    };

    juce::Array<TopicLayout> layouts;
    int preferredHeight { 100 };

    //==========================================================================
    void recalcLayouts()
    {
        layouts.clear();
        int y = kTopMargin;

        // Dynamic line heights based on current text font size.
        const int titleH    = juce::jmax (kTopicTitleH, (int)(textFontSize + 16.0f));
        const int textLineH = juce::jmax (16, (int)(textFontSize + 7.0f));

        for (int ti = 0; ti < lesson.topics.size(); ++ti)
        {
            const auto& topic = lesson.topics.getReference (ti);
            if (ti > 0) y += kTopicGap;

            TopicLayout tl;
            tl.y = y;

            y += titleH;
            y += topic.textLines.size() * textLineH;

            tl.scoreY = y;

            if (!topic.score.isEmpty())
            {
                // Add score area height for each display line + "try again" gap.
                const int numLines = countScoreLines (topic.score);
                y += numLines * kScoreAreaH + kTryAgainH;
            }

            tl.height = y - tl.y;
            layouts.add (tl);
        }

        y += kTopMargin;
        preferredHeight = juce::jmax (100, y);
    }

    //==========================================================================
    static juce::Colour rowColour (int row)
    {
        switch (row)
        {
            case Proc::COUNTERBASS: return juce::Colour (0xffb0b0b0);
            case Proc::BASS:        return juce::Colour (0xffffffff);
            case Proc::MAJOR:       return juce::Colour (0xffffb347);
            case Proc::MINOR:       return juce::Colour (0xff6699ff);
            default:                return juce::Colours::grey;
        }
    }

    static juce::String rowAbbrev (int row)
    {
        switch (row)
        {
            case Proc::COUNTERBASS: return "3rd";
            case Proc::BASS:        return "Bs";
            case Proc::MAJOR:       return "Maj";
            case Proc::MINOR:       return "Min";
            default:                return "?";
        }
    }

    //==========================================================================
    // Layout constants (pixels)
    static constexpr int kMargin      = 12;
    static constexpr int kTopMargin   = 12;
    static constexpr int kTopicGap    = 28;
    static constexpr int kTopicTitleH = 28;   ///< minimum topic-title row height (scales with textFontSize)
    static constexpr int kTextLineH   = 20;   ///< minimum text-line height; actual value computed from textFontSize
    static constexpr int kScoreAreaH  = 58;   ///< height of one score display line
    static constexpr int kScoreNoteW  = 50;
    static constexpr int kScoreNoteH  = 40;
    static constexpr int kPlayBtnSize = 26;
    static constexpr int kScoreGap    = 10;
    static constexpr int kScoreNoteGap=  4;
    static constexpr int kScoreSpaceW = 24;   ///< extra x-gap inserted by a '-' group separator
    static constexpr int kTryAgainH   = 22;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LessonPanel)
};


//==============================================================================
// LessonsWindow implementation
//==============================================================================

LessonsWindow::LessonsWindow (StraDellaMIDI_pluginAudioProcessor& p)
    : audioProcessor (p)
{
    setWantsKeyboardFocus (true);

    // ── Lesson selector ───────────────────────────────────────────────────────
    selectorLabel.setText ("Lesson:", juce::dontSendNotification);
    selectorLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    selectorLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (selectorLabel);

    lessonSelector.onChange = [this]
    {
        showLesson (lessonSelector.getSelectedId() - 1);
    };
    addAndMakeVisible (lessonSelector);

    // ── Viewport + lesson panel ───────────────────────────────────────────────
    panel = std::make_unique<LessonPanel>();
    panel->onPlayButtonClicked = [this] (int topicIdx) { startPlayback (topicIdx); };

    viewport.setScrollBarsShown (true, false);   // vertical scroll only
    viewport.setViewedComponent (panel.get(), false);
    addAndMakeVisible (viewport);

    // ── Text-size buttons (A- / A+) ───────────────────────────────────────────
    textSmallerButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a3a5e));
    textSmallerButton.setTooltip ("Decrease text size");
    textSmallerButton.onClick = [this]
    {
        textFontSize = juce::jmax (10.0f, textFontSize - 1.0f);
        panel->setTextFontSize (textFontSize);
        const int pw = viewport.getMaximumVisibleWidth();
        if (pw > 0) panel->setSize (pw, panel->getPreferredHeight());
    };
    addAndMakeVisible (textSmallerButton);

    textLargerButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a3a5e));
    textLargerButton.setTooltip ("Increase text size");
    textLargerButton.onClick = [this]
    {
        textFontSize = juce::jmin (20.0f, textFontSize + 1.0f);
        panel->setTextFontSize (textFontSize);
        const int pw = viewport.getMaximumVisibleWidth();
        if (pw > 0) panel->setSize (pw, panel->getPreferredHeight());
    };
    addAndMakeVisible (textLargerButton);

    // ── Close button ──────────────────────────────────────────────────────────
    closeButton.onClick = [this]
    {
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->closeButtonPressed();
        else
            setVisible (false);
    };
    addAndMakeVisible (closeButton);

    // ── Load lessons & populate selector ─────────────────────────────────────
    loadLessons();

    setSize (600, 520);
    grabKeyboardFocus();
}

LessonsWindow::~LessonsWindow()
{
    stopTimer();
    // Release any notes we may have pressed during playback
    if (playbackTopicIdx >= 0 && playbackPhase && currentLessonIdx >= 0)
    {
        const auto& topic = lessons[currentLessonIdx].topics[playbackTopicIdx];
        const int si = findNthNote (topic.score, playbackNoteIdx);
        if (si >= 0)
            audioProcessor.buttonReleased (topic.score[si].row, topic.score[si].col);
    }
    // Release any keyboard-held notes
    for (auto it = activeKeyRow.begin(); it != activeKeyRow.end(); ++it)
        audioProcessor.buttonReleased (it.getValue(), activeKeyCol[it.getKey()]);
}

//==============================================================================
void LessonsWindow::loadLessons()
{
    lessons.clear();
    lessonSelector.clear();

    // 1. Load built-in lessons
    const juce::String builtIn = getBuiltInLessonsText();
    if (builtIn.isNotEmpty())
    {
        for (const auto& ld : parseLessons (builtIn))
            lessons.add (ld);
    }

    // 2. Also try loading .lesson files from a user directory
    const juce::File userDir = juce::File::getSpecialLocation (
        juce::File::userApplicationDataDirectory)
        .getChildFile ("straDellaMIDI/lessons");

    if (userDir.isDirectory())
    {
        for (const auto& f : userDir.findChildFiles (juce::File::findFiles, false, "*.lesson"))
        {
            const juce::String content = f.loadFileAsString();
            for (const auto& ld : parseLessons (content))
                lessons.add (ld);
        }
    }

    // Populate drop-down
    for (int i = 0; i < lessons.size(); ++i)
        lessonSelector.addItem (lessons[i].title, i + 1);

    if (!lessons.isEmpty())
    {
        lessonSelector.setSelectedId (1, juce::dontSendNotification);
        showLesson (0);
    }
}

//==============================================================================
void LessonsWindow::showLesson (int index)
{
    if (index < 0 || index >= lessons.size()) return;

    stopPlayback();
    currentLessonIdx = index;
    userTopicIdx     = 0;
    userNoteIdx      = 0;
    tryAgain         = false;
    activeKeyRow.clear();
    activeKeyCol.clear();

    panel->setLesson (lessons[index]);
    refreshPanel();

    // Size the panel to fit its content
    const int pw = viewport.getMaximumVisibleWidth();
    panel->setSize (pw, panel->getPreferredHeight());

    scrollToCurrentTopic();
    grabKeyboardFocus();
}

//==============================================================================
void LessonsWindow::refreshPanel()
{
    LessonPanel::State s;
    s.playbackTopicIdx   = playbackTopicIdx;
    s.highlightedNoteIdx = highlightedNoteIdx;
    s.userTopicIdx       = userTopicIdx;
    s.userNoteIdx        = userNoteIdx;
    s.tryAgain           = tryAgain;
    s.flashOn            = flashOn;
    panel->setState (s);

    // Keep the panel sized correctly (height can change after state update)
    const int pw = viewport.getMaximumVisibleWidth();
    if (pw > 0)
        panel->setSize (pw, panel->getPreferredHeight());
}

//==============================================================================
void LessonsWindow::scrollToCurrentTopic()
{
    if (panel == nullptr) return;
    const int y = panel->getTopicY (userTopicIdx);
    viewport.setViewPosition (0, juce::jmax (0, y - 10));
}

//==============================================================================
// Layout
//==============================================================================
void LessonsWindow::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));
    g.setColour (juce::Colour (0xff404060));
    g.drawRect (getLocalBounds(), 1);
}

void LessonsWindow::resized()
{
    auto area = getLocalBounds().reduced (kMargin);

    // Top: label + combo + A-/A+ text-size buttons
    auto topRow = area.removeFromTop (kComboH);
    selectorLabel.setBounds (topRow.removeFromLeft (55));
    topRow.removeFromLeft (4);
    // A-/A+ buttons sit on the right of the top row
    textLargerButton .setBounds (topRow.removeFromRight (32));
    topRow.removeFromRight (2);
    textSmallerButton.setBounds (topRow.removeFromRight (32));
    topRow.removeFromRight (4);
    lessonSelector.setBounds (topRow);
    area.removeFromTop (8);

    // Bottom: close button
    auto bottomRow = area.removeFromBottom (kCloseBtnH);
    closeButton.setBounds (bottomRow.withSizeKeepingCentre (80, kCloseBtnH));
    area.removeFromBottom (6);

    // Middle: viewport
    viewport.setBounds (area);

    // Resize the panel content to viewport's visible width
    if (panel != nullptr)
    {
        const int pw = viewport.getMaximumVisibleWidth();
        if (pw > 0)
            panel->setSize (pw, panel->getPreferredHeight());
    }
}

//==============================================================================
// Score playback
//==============================================================================
void LessonsWindow::startPlayback (int topicIdx)
{
    stopPlayback();

    if (currentLessonIdx < 0 || topicIdx < 0
        || topicIdx >= lessons[currentLessonIdx].topics.size()) return;

    if (lessons[currentLessonIdx].topics[topicIdx].score.isEmpty()) return;

    playbackTopicIdx  = topicIdx;
    playbackNoteIdx   = 0;
    playbackPhase     = false;   // next action: note-on
    highlightedNoteIdx = -1;
    timerMode         = TimerMode::Playback;

    onPlayTick();                          // press first note immediately
    startTimer (kPlayTimerMs);
}

void LessonsWindow::stopPlayback()
{
    if (timerMode == TimerMode::Playback || timerMode == TimerMode::Flash)
    {
        stopTimer();

        // Release the note currently held during playback
        if (timerMode == TimerMode::Playback
            && playbackTopicIdx >= 0 && playbackPhase
            && currentLessonIdx >= 0)
        {
            const auto& topic = lessons[currentLessonIdx].topics[playbackTopicIdx];
            const int si = findNthNote (topic.score, playbackNoteIdx);
            if (si >= 0)
                audioProcessor.buttonReleased (topic.score[si].row, topic.score[si].col);
        }
    }

    timerMode          = TimerMode::Idle;
    const int prevTopic = playbackTopicIdx;
    playbackTopicIdx   = -1;
    playbackNoteIdx    =  0;
    playbackPhase      = false;
    highlightedNoteIdx = -1;

    const bool hadFlash = flashOn || flashCount > 0;
    flashOn            = false;
    flashCount         =  0;

    if (prevTopic >= 0 || hadFlash)
        refreshPanel();
}

void LessonsWindow::timerCallback()
{
    if (timerMode == TimerMode::Playback)
        onPlayTick();
    else if (timerMode == TimerMode::Flash)
        onFlashTick();
}

void LessonsWindow::onPlayTick()
{
    if (currentLessonIdx < 0 || playbackTopicIdx < 0)
    {
        stopPlayback();
        return;
    }

    const auto& topic = lessons[currentLessonIdx].topics[playbackTopicIdx];

    if (playbackNoteIdx >= countNotes (topic.score))
    {
        stopPlayback();
        return;
    }

    const int si = findNthNote (topic.score, playbackNoteIdx);
    if (si < 0)
    {
        stopPlayback();
        return;
    }

    const auto& note = topic.score[si];

    if (!playbackPhase)
    {
        // ── Note-on ──────────────────────────────────────────────────────────
        audioProcessor.buttonPressed (note.row, note.col, 100);
        highlightedNoteIdx = playbackNoteIdx;   // actual note index
        playbackPhase = true;
    }
    else
    {
        // ── Note-off ─────────────────────────────────────────────────────────
        audioProcessor.buttonReleased (note.row, note.col);
        highlightedNoteIdx = -1;
        ++playbackNoteIdx;
        playbackPhase = false;

        if (playbackNoteIdx >= countNotes (topic.score))
        {
            // All notes played – stop playback cleanly
            const int prevTopic = playbackTopicIdx;
            timerMode          = TimerMode::Idle;
            stopTimer();
            playbackTopicIdx   = -1;
            playbackNoteIdx    =  0;
            highlightedNoteIdx = -1;
            flashOn            = false;
            flashCount         =  0;
            if (prevTopic >= 0) refreshPanel();
            return;
        }
    }

    refreshPanel();
}

void LessonsWindow::onFlashTick()
{
    flashOn = !flashOn;
    ++flashCount;
    refreshPanel();

    if (flashCount >= 6)
    {
        // Three full flashes done – advance to next topic
        stopTimer();
        timerMode = TimerMode::Idle;
        flashOn   = false;
        flashCount = 0;
        advanceTopic();
    }
}

//==============================================================================
// User input handling
//==============================================================================
bool LessonsWindow::keyPressed (const juce::KeyPress& key)
{
    const int keyCode = key.getKeyCode();

    // Ignore repeated key events
    if (activeKeyRow.contains (keyCode))
        return true;

    // Map key code to stradella grid coordinates
    int row = -1, col = -1;

    // The default mapping (same as StradellaKeyboardMapper::setupDefaultMappings):
    //   Bass (row 1):         q  w  e  r  t  y  u  i  o  p   (cols 0-9)
    //   Counterbass (row 0):  1  2  3  4  5  6  7  8  9  0   (cols 0-9)
    //   Major (row 2):        a  s  d  f  g  h  j  k  l  ;   (cols 0-9)
    //   Minor (row 3):        z  x  c  v  b  n  m  ,  .  /   (cols 0-9)
    static const int bassKeys[]  = { 'q','w','e','r','t','y','u','i','o','p' };
    static const int cbKeys[]    = { '1','2','3','4','5','6','7','8','9','0' };
    static const int majorKeys[] = { 'a','s','d','f','g','h','j','k','l',';' };
    static const int minorKeys[] = { 'z','x','c','v','b','n','m',',','.','/' };

    const int lowerCode = (keyCode >= 'A' && keyCode <= 'Z')
                        ? keyCode + ('a' - 'A')
                        : keyCode;

    for (int i = 0; i < 10; ++i)
    {
        if (lowerCode == bassKeys [i]) { row = 1; col = i; break; }
        if (lowerCode == cbKeys   [i]) { row = 0; col = i; break; }
        if (lowerCode == majorKeys[i]) { row = 2; col = i; break; }
        if (lowerCode == minorKeys[i]) { row = 3; col = i; break; }
    }

    if (row < 0)
        return false;

    activeKeyRow.set (keyCode, row);
    activeKeyCol.set (keyCode, col);

    audioProcessor.buttonPressed (row, col, 100);
    handleNotePress (row, col);
    return true;
}

bool LessonsWindow::keyStateChanged (bool isKeyDown)
{
    if (isKeyDown)
        return false;

    juce::Array<int> toRelease;
    for (auto it = activeKeyRow.begin(); it != activeKeyRow.end(); ++it)
        if (!juce::KeyPress::isKeyCurrentlyDown (it.getKey()))
            toRelease.add (it.getKey());

    for (int kc : toRelease)
    {
        const int row = activeKeyRow[kc];
        const int col = activeKeyCol[kc];
        audioProcessor.buttonReleased (row, col);
        handleNoteRelease (row, col);
        activeKeyRow.remove (kc);
        activeKeyCol.remove (kc);
    }

    return !toRelease.isEmpty();
}

void LessonsWindow::handleNotePress (int row, int col)
{
    if (currentLessonIdx < 0) return;
    const auto& lesson = lessons[currentLessonIdx];
    if (userTopicIdx >= lesson.topics.size()) return;

    const auto& topic = lesson.topics[userTopicIdx];
    if (topic.score.isEmpty()) return;

    // Already flash-animating a correct answer – ignore input
    if (timerMode == TimerMode::Flash) return;

    const int si = findNthNote (topic.score, userNoteIdx);
    if (si < 0) return;

    const LessonScoreNote pressed { row, col };
    const auto& expected = topic.score[si];

    if (pressed == expected)
    {
        ++userNoteIdx;
        tryAgain = false;

        if (userNoteIdx >= countNotes (topic.score))
            onScoreComplete();
        else
            refreshPanel();
    }
    else
    {
        // Wrong note
        userNoteIdx = 0;
        tryAgain    = true;
        refreshPanel();
    }
}

void LessonsWindow::handleNoteRelease (int /*row*/, int /*col*/)
{
    // Nothing extra needed on release for score matching
}

void LessonsWindow::onScoreComplete()
{
    // Start flash animation
    stopTimer();
    timerMode  = TimerMode::Flash;
    flashOn    = true;
    flashCount = 0;
    refreshPanel();
    startTimer (kFlashTimerMs);
}

void LessonsWindow::advanceTopic()
{
    if (currentLessonIdx < 0) return;
    const auto& lesson = lessons[currentLessonIdx];

    ++userTopicIdx;
    userNoteIdx = 0;
    tryAgain    = false;
    flashOn     = false;
    flashCount  = 0;

    if (userTopicIdx >= lesson.topics.size())
    {
        // Lesson complete – show a congratulatory message by resetting to first topic
        userTopicIdx = 0;
    }

    refreshPanel();
    scrollToCurrentTopic();
}

//==============================================================================
// Lesson markup parser
//==============================================================================
juce::Array<LessonData> LessonsWindow::parseLessons (const juce::String& text)
{
    juce::Array<LessonData> result;
    LessonData  current;
    LessonTopic currentTopic;
    bool hasTopic   = false;
    bool hasLesson  = false;

    // Column names matching PluginProcessor::kRootNotes order
    static const char* colNames[] = { "Eb","Bb","F","C","G","D","A","E","B","F#","Db","Ab" };

    auto pushTopic = [&]()
    {
        if (hasTopic)
        {
            current.topics.add (currentTopic);
            currentTopic = LessonTopic{};
            hasTopic = false;
        }
    };

    auto pushLesson = [&]()
    {
        pushTopic();
        if (hasLesson && current.title.isNotEmpty())
            result.add (current);
        current   = LessonData{};
        hasLesson = false;
    };

    const juce::StringArray lines = juce::StringArray::fromLines (text);

    for (const auto& rawLine : lines)
    {
        const auto line = rawLine.trim();

        // Comment or empty
        if (line.isEmpty() || line.startsWith ("#"))
            continue;

        if (line.startsWith ("LESSON:"))
        {
            pushLesson();
            current.title = line.substring (7).trim();
            hasLesson = true;
        }
        else if (line.startsWith ("DESCRIPTION:"))
        {
            current.description = line.substring (12).trim();
        }
        else if (line.startsWith ("TOPIC:"))
        {
            pushTopic();
            currentTopic.title = line.substring (6).trim();
            hasTopic = true;
        }
        else if (line.startsWith ("TEXT:"))
        {
            currentTopic.textLines.add (line.substring (5).trim());
        }
        else if (line.startsWith ("SCORE:"))
        {
            const auto scoreStr = line.substring (6).trim();
            juce::Array<LessonScoreNote> score;

            if (scoreStr.containsChar (':'))
            {
                // ── Legacy format: "rowname:colname" tokens separated by spaces ──
                // e.g.  bass:C  major:G  minor:A  bass:F
                juce::StringArray tokens;
                tokens.addTokens (scoreStr, " ", "");

                for (const auto& token : tokens)
                {
                    const int colon = token.indexOf (":");
                    if (colon <= 0) continue;

                    const auto rowStr = token.substring (0, colon).trim().toLowerCase();
                    const auto colStr = token.substring (colon + 1).trim();

                    int row = -1;
                    if      (rowStr == "third" || rowStr == "counterbass") row = 0;
                    else if (rowStr == "bass")                             row = 1;
                    else if (rowStr == "major")                            row = 2;
                    else if (rowStr == "minor")                            row = 3;

                    int col = -1;
                    for (int c = 0; c < 12; ++c)
                        if (colStr.equalsIgnoreCase (colNames[c])) { col = c; break; }

                    if (row >= 0 && col >= 0)
                        score.add ({ row, col });
                }
            }
            else
            {
                // ── Compact format: keyboard characters + layout markers ───────
                // Each character is the keyboard key that plays the note/chord.
                //   -  (dash)  → group-space sentinel
                //   |  (pipe)  → line-break sentinel
                // e.g.  rftf-rftf|tgyg-tgyg
                static const int bassKeys[]  = { 'q','w','e','r','t','y','u','i','o','p' };
                static const int cbKeys[]    = { '1','2','3','4','5','6','7','8','9','0' };
                static const int majorKeys[] = { 'a','s','d','f','g','h','j','k','l',';' };
                static const int minorKeys[] = { 'z','x','c','v','b','n','m',',','.','/' };

                for (int ci = 0; ci < scoreStr.length(); ++ci)
                {
                    const int ch = (int)(juce::juce_wchar) scoreStr[ci];

                    if (ch == '-') { score.add ({ -2, -1 }); continue; }  // group-space
                    if (ch == '|') { score.add ({ -3, -1 }); continue; }  // line-break
                    if (ch == ' ') continue;                               // ignored

                    const int lower = (ch >= 'A' && ch <= 'Z') ? ch + ('a' - 'A') : ch;
                    int row = -1, col = -1;
                    for (int ki = 0; ki < 10; ++ki)
                    {
                        if (lower == bassKeys [ki]) { row = 1; col = ki; break; }
                        if (lower == cbKeys   [ki]) { row = 0; col = ki; break; }
                        if (lower == majorKeys[ki]) { row = 2; col = ki; break; }
                        if (lower == minorKeys[ki]) { row = 3; col = ki; break; }
                    }

                    if (row >= 0 && col >= 0)
                        score.add ({ row, col });
                }
            }

            if (!score.isEmpty())
                currentTopic.score = score;
        }
    }

    pushLesson();
    return result;
}

//==============================================================================
// Built-in lesson content
//==============================================================================
juce::String LessonsWindow::getBuiltInLessonsText()
{
    return juce::String (
        "# straDellaMIDI Built-in Lessons\n"
        "# ================================\n"
        "# Format: LESSON / DESCRIPTION / TOPIC / TEXT / SCORE\n"
        "# SCORE: keyboard characters map directly to accordion buttons.\n"
        "#   - (dash) separates note groups; | (pipe) starts a new score line.\n"
        "#   e.g.  SCORE: rf-tg   means bass:C chord:C – bass:G chord:G\n"
        "# Legacy colon format still accepted: SCORE: bass:C major:G minor:A\n"
        "\n"
        "LESSON: Getting Started\n"
        "DESCRIPTION: An introduction to the Stradella bass system\n"
        "\n"
        "TOPIC: Welcome\n"
        "TEXT: Welcome to straDellaMIDI lessons!\n"
        "TEXT: Each button shows its keyboard key. Hover over it to reveal the note name.\n"
        "TEXT: Press the triangle (>) to hear a sequence, then play it yourself!\n"
        "TEXT: When you play a note correctly it reveals its name; complete the score to keep them.\n"
        "SCORE: rter\n"
        "\n"
        "TOPIC: The Bass Row\n"
        "TEXT: The bass row plays the root note of each key.\n"
        "TEXT: Keyboard: q=Eb  w=Bb  e=F  r=C  t=G  y=D  u=A  i=E  o=B  p=F#\n"
        "SCORE: qwer\n"
        "\n"
        "TOPIC: More Bass Notes\n"
        "TEXT: Continue across the circle of fifths.\n"
        "SCORE: tyuio\n"
        "\n"
        "LESSON: Major Chords\n"
        "DESCRIPTION: Playing major chords with the orange row\n"
        "\n"
        "TOPIC: Introduction to Major Chords\n"
        "TEXT: The orange row plays a major triad: root, major 3rd, and perfect 5th.\n"
        "TEXT: Keyboard: a=Eb  s=Bb  d=F  f=C  g=G  h=D  j=A  k=E  l=B  ;=F#\n"
        "SCORE: fgdf\n"
        "\n"
        "TOPIC: I-IV-V in C major\n"
        "TEXT: C, F, and G major are the I, IV, and V chords in the key of C.\n"
        "TEXT: This progression is used in countless songs!\n"
        "SCORE: fdgf\n"
        "\n"
        "TOPIC: Practice\n"
        "TEXT: Play these four major chords in sequence.\n"
        "SCORE: fhkg\n"
        "\n"
        "LESSON: Minor Chords\n"
        "DESCRIPTION: Playing minor chords with the blue row\n"
        "\n"
        "TOPIC: Introduction to Minor Chords\n"
        "TEXT: The blue row plays a minor triad: root, minor 3rd, and perfect 5th.\n"
        "TEXT: Keyboard: z=Eb  x=Bb  c=F  v=C  b=G  n=D  m=A  ,=E  .=B  /=F#\n"
        "SCORE: mn,m\n"
        "\n"
        "TOPIC: Am-Dm-Em Progression\n"
        "TEXT: A minor, D minor, and E minor are common in the key of A minor.\n"
        "SCORE: mn,m\n"
        "\n"
        "LESSON: Mixing Bass and Chords\n"
        "DESCRIPTION: Combining bass notes with chords\n"
        "\n"
        "TOPIC: Bass then Chord\n"
        "TEXT: A common accordion pattern is to alternate bass notes with chords.\n"
        "TEXT: Try: C bass, then C major, then G bass, then G major.\n"
        "SCORE: rf-tg\n"
        "\n"
        "TOPIC: Simple Waltz Pattern\n"
        "TEXT: The waltz uses one bass note then two chords per bar.\n"
        "TEXT: Try the C waltz: C bass, C major, C major.\n"
        "SCORE: rff-tgg\n"
        "\n"
        "TOPIC: Full I-IV-V Accompaniment\n"
        "TEXT: Mix bass notes and chords for a complete I-IV-V progression.\n"
        "TEXT: The dash (-) separates beat groups; the pipe (|) separates bars.\n"
        "SCORE: rf-ed|tg-rf\n"
    );
}
