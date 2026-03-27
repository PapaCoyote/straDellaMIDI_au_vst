/*
  ==============================================================================

    StraDellaMIDI – Stradella Bass Accordion MIDI Effect Plugin
    Lessons window implementation.

  ==============================================================================
*/

#include "LessonsWindow.h"

using Proc = StraDellaMIDI_pluginAudioProcessor;

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

        const juce::Font titleFont (juce::FontOptions (14.0f, juce::Font::bold));
        const juce::Font textFont  (juce::FontOptions (12.0f));
        const juce::Font noteFont  (juce::FontOptions (11.0f, juce::Font::bold));
        const juce::Font smallFont (juce::FontOptions (9.0f));
        const juce::Font keyFont   (juce::FontOptions (13.0f, juce::Font::bold));

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
                        kMargin, layout.y, getWidth() - 2 * kMargin, kTopicTitleH,
                        juce::Justification::centredLeft);

            // ── Text lines ────────────────────────────────────────────────────
            g.setColour (juce::Colour (0xffccccdd));
            g.setFont (textFont);
            for (int li = 0; li < topic.textLines.size(); ++li)
            {
                g.drawText (topic.textLines[li],
                            kMargin + 4,
                            layout.y + kTopicTitleH + li * kTextLineH,
                            getWidth() - 2 * kMargin - 8, kTextLineH,
                            juce::Justification::centredLeft);
            }

            // ── Score area ────────────────────────────────────────────────────
            if (!topic.score.isEmpty())
            {
                const int  scoreY     = layout.scoreY;
                const bool isThisTopic = (state.playbackTopicIdx == ti);
                const bool isFlashing  = (state.flashOn
                                          && state.userTopicIdx == ti
                                          && state.userNoteIdx >= topic.score.size());

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
                for (int ni = 0; ni < topic.score.size(); ++ni)
                {
                    const auto& note = topic.score.getReference (ni);
                    const int   nx   = kMargin + kPlayBtnSize + kScoreGap
                                       + ni * (kScoreNoteW + kScoreNoteGap);
                    const int   ny   = scoreY + (kScoreAreaH - kScoreNoteH) / 2;

                    const bool isHighlighted = isThisTopic && (state.highlightedNoteIdx == ni);
                    const bool isPlayed      = (ti < state.userTopicIdx)
                                           || (ti == state.userTopicIdx && ni < state.userNoteIdx);

                    const int expand = isHighlighted ? 5 : 0;
                    const juce::Rectangle<int> noteBounds (
                        nx - expand / 2, ny - expand / 2,
                        kScoreNoteW + expand, kScoreNoteH + expand);

                    juce::Colour nc = rowColour (note.row);
                    if (isFlashing)        nc = nc.brighter (0.8f);
                    else if (isPlayed)     nc = nc.withAlpha (0.45f);

                    g.setColour (nc);
                    g.fillRoundedRectangle (noteBounds.toFloat(), 4.0f);

                    g.setColour (isHighlighted ? juce::Colours::white
                                               : juce::Colour (0xff505050));
                    g.drawRoundedRectangle (noteBounds.toFloat(), 4.0f, 1.0f);

                    // Label: keyboard key when highlighted, note name otherwise
                    g.setColour (juce::Colours::black);
                    const juce::String label = isHighlighted
                                             ? getKeyChar (note.row, note.col)
                                             : Proc::getColumnName (note.col);
                    g.setFont (isHighlighted ? keyFont : noteFont);
                    g.drawFittedText (label,
                                      noteBounds.withTrimmedBottom (isHighlighted ? 0 : 10)
                                               .reduced (2),
                                      juce::Justification::centred, 1);

                    // Row-type abbreviation (small text at bottom of note box)
                    if (!isHighlighted)
                    {
                        g.setFont (smallFont);
                        g.setColour (juce::Colours::black.withAlpha (0.55f));
                        g.drawFittedText (rowAbbrev (note.row),
                                          juce::Rectangle<int> (nx, ny + kScoreNoteH - 11,
                                                                 kScoreNoteW, 11),
                                          juce::Justification::centred, 1);
                    }
                }

                // ── "Try again" message ───────────────────────────────────────
                if (state.tryAgain && state.userTopicIdx == ti)
                {
                    g.setColour (juce::Colour (0xffff7777));
                    g.setFont (textFont);
                    g.drawText ("Try again",
                                kMargin + kPlayBtnSize + kScoreGap,
                                scoreY + kScoreAreaH + 2,
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

        for (int ti = 0; ti < lesson.topics.size(); ++ti)
        {
            const auto& topic = lesson.topics.getReference (ti);
            if (ti > 0) y += kTopicGap;

            TopicLayout tl;
            tl.y = y;

            y += kTopicTitleH;
            y += topic.textLines.size() * kTextLineH;

            tl.scoreY = y;

            if (!topic.score.isEmpty())
            {
                // Add score area height + small gap for "Try again" message
                y += kScoreAreaH + kTryAgainH;
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

    static juce::String getKeyChar (int row, int col)
    {
        if (col < 0 || col > 9) return {};

        static const juce::String bassKeys[]  = { "q","w","e","r","t","y","u","i","o","p" };
        static const juce::String cbKeys[]    = { "1","2","3","4","5","6","7","8","9","0" };
        static const juce::String majorKeys[] = { "a","s","d","f","g","h","j","k","l",";" };
        static const juce::String minorKeys[] = { "z","x","c","v","b","n","m",",",".","/"}; // col 9 = F#

        switch (row)
        {
            case 0: return cbKeys   [col];
            case 1: return bassKeys [col];
            case 2: return majorKeys[col];
            case 3: return minorKeys[col];
            default: return {};
        }
    }

    //==========================================================================
    // Layout constants (pixels)
    static constexpr int kMargin      = 12;
    static constexpr int kTopMargin   = 12;
    static constexpr int kTopicGap    = 28;
    static constexpr int kTopicTitleH = 28;
    static constexpr int kTextLineH   = 20;
    static constexpr int kScoreAreaH  = 58;
    static constexpr int kScoreNoteW  = 50;
    static constexpr int kScoreNoteH  = 40;
    static constexpr int kPlayBtnSize = 26;
    static constexpr int kScoreGap    = 10;
    static constexpr int kScoreNoteGap=  4;
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
        if (playbackNoteIdx < topic.score.size())
            audioProcessor.buttonReleased (topic.score[playbackNoteIdx].row,
                                           topic.score[playbackNoteIdx].col);
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

    // Top: label + combo
    auto topRow = area.removeFromTop (kComboH);
    selectorLabel.setBounds (topRow.removeFromLeft (55));
    topRow.removeFromLeft (4);
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
            if (playbackNoteIdx < topic.score.size())
                audioProcessor.buttonReleased (topic.score[playbackNoteIdx].row,
                                               topic.score[playbackNoteIdx].col);
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

    if (playbackNoteIdx >= topic.score.size())
    {
        stopPlayback();
        return;
    }

    const auto& note = topic.score[playbackNoteIdx];

    if (!playbackPhase)
    {
        // ── Note-on ──────────────────────────────────────────────────────────
        audioProcessor.buttonPressed (note.row, note.col, 100);
        highlightedNoteIdx = playbackNoteIdx;
        playbackPhase = true;
    }
    else
    {
        // ── Note-off ─────────────────────────────────────────────────────────
        audioProcessor.buttonReleased (note.row, note.col);
        highlightedNoteIdx = -1;
        ++playbackNoteIdx;
        playbackPhase = false;

        if (playbackNoteIdx >= topic.score.size())
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

    const LessonScoreNote pressed { row, col };
    const auto& expected = topic.score[userNoteIdx];

    if (pressed == expected)
    {
        ++userNoteIdx;
        tryAgain = false;

        if (userNoteIdx >= topic.score.size())
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
            juce::StringArray tokens;
            tokens.addTokens (scoreStr, " ", "");

            juce::Array<LessonScoreNote> score;
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
        "# SCORE tokens: rowname:colname  (e.g. bass:C  major:G  minor:A)\n"
        "# rowname : third | bass | major | minor\n"
        "# colname : Eb Bb F C G D A E B F# Db Ab\n"
        "\n"
        "LESSON: Getting Started\n"
        "DESCRIPTION: An introduction to the Stradella bass system\n"
        "\n"
        "TOPIC: Welcome\n"
        "TEXT: Welcome to straDellaMIDI lessons!\n"
        "TEXT: The Stradella bass system is used on the left hand of the accordion.\n"
        "TEXT: You will learn bass notes, major chords, and minor chords.\n"
        "TEXT: Press the triangle (>) to hear a sequence, then play it yourself!\n"
        "SCORE: bass:C bass:G bass:F bass:C\n"
        "\n"
        "TOPIC: The Bass Row\n"
        "TEXT: The bass row plays the root note of each key.\n"
        "TEXT: Keyboard: q=Eb  w=Bb  e=F  r=C  t=G  y=D  u=A  i=E  o=B  p=F#\n"
        "SCORE: bass:Eb bass:Bb bass:F bass:C\n"
        "\n"
        "TOPIC: More Bass Notes\n"
        "TEXT: Continue across the circle of fifths.\n"
        "SCORE: bass:G bass:D bass:A bass:E bass:B\n"
        "\n"
        "LESSON: Major Chords\n"
        "DESCRIPTION: Playing major chords with the orange row\n"
        "\n"
        "TOPIC: Introduction to Major Chords\n"
        "TEXT: The orange row plays a major triad: root, major 3rd, and perfect 5th.\n"
        "TEXT: Keyboard: a=Eb  s=Bb  d=F  f=C  g=G  h=D  j=A  k=E  l=B  ;=F#\n"
        "SCORE: major:C major:G major:F major:C\n"
        "\n"
        "TOPIC: I-IV-V in C major\n"
        "TEXT: C, F, and G major are the I, IV, and V chords in the key of C.\n"
        "TEXT: This progression is used in countless songs!\n"
        "SCORE: major:C major:F major:G major:C\n"
        "\n"
        "TOPIC: Practice\n"
        "TEXT: Play these four major chords in sequence.\n"
        "SCORE: major:C major:D major:E major:G\n"
        "\n"
        "LESSON: Minor Chords\n"
        "DESCRIPTION: Playing minor chords with the blue row\n"
        "\n"
        "TOPIC: Introduction to Minor Chords\n"
        "TEXT: The blue row plays a minor triad: root, minor 3rd, and perfect 5th.\n"
        "TEXT: Keyboard: z=Eb  x=Bb  c=F  v=C  b=G  n=D  m=A  ,=E  .=B  /=F#\n"
        "SCORE: minor:A minor:D minor:E minor:A\n"
        "\n"
        "TOPIC: Am-Dm-Em Progression\n"
        "TEXT: A minor, D minor, and E minor are common in the key of A minor.\n"
        "SCORE: minor:A minor:D minor:E minor:A\n"
        "\n"
        "LESSON: Mixing Bass and Chords\n"
        "DESCRIPTION: Combining bass notes with chords\n"
        "\n"
        "TOPIC: Bass then Chord\n"
        "TEXT: A common accordion pattern is to alternate bass notes with chords.\n"
        "TEXT: Try: C bass, then C major, then G bass, then G major.\n"
        "SCORE: bass:C major:C bass:G major:G\n"
        "\n"
        "TOPIC: Simple Waltz Pattern\n"
        "TEXT: The waltz uses one bass note then two chords per bar.\n"
        "TEXT: Try the C waltz: C bass, C major, C major.\n"
        "SCORE: bass:C major:C major:C bass:G major:G major:G\n"
    );
}

//==============================================================================
// Static helper: keyboard key character for a given (row, col) pair
//==============================================================================
juce::String LessonsWindow::getKeyChar (int row, int col)
{
    if (col < 0 || col > 9) return {};

    static const juce::String bassKeys[]  = { "q","w","e","r","t","y","u","i","o","p" };
    static const juce::String cbKeys[]    = { "1","2","3","4","5","6","7","8","9","0" };
    static const juce::String majorKeys[] = { "a","s","d","f","g","h","j","k","l",";" };
    static const juce::String minorKeys[] = { "z","x","c","v","b","n","m",",",".","/"}; // col 9 = F#

    switch (row)
    {
        case 0: return cbKeys   [col];
        case 1: return bassKeys [col];
        case 2: return majorKeys[col];
        case 3: return minorKeys[col];
        default: return {};
    }
}
