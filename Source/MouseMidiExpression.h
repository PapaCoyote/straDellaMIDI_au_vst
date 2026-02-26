#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Handles mouse-based MIDI expression control, emulating accordion bellows.
    - Mouse Y position determines note velocity (127 at top, 0 at bottom)
    - Mouse Y position determines CC1 and CC11 (only when moving in X direction)
    - CC1 and CC11 decay to 0 when X movement stops
    - X direction changes trigger note off/on for all pressed keys
    
    Uses global mouse tracking to monitor movement across the entire desktop.
*/
class MouseMidiExpression : private juce::Timer
{
public:
    //==============================================================================
    /** Curve types for mapping mouse movement to MIDI values */
    enum class CurveType
    {
        Linear,
        Exponential,
        Logarithmic
    };
    
    //==============================================================================
    MouseMidiExpression();
    ~MouseMidiExpression() override;
    
    /** Sets whether CC1 (Modulation Wheel) is enabled */
    void setModulationEnabled(bool enabled) { modulationEnabled = enabled; }
    
    /** Sets whether CC11 (Expression) is enabled */
    void setExpressionEnabled(bool enabled) { expressionEnabled = enabled; }
    
    /** Sets the curve type for mapping */
    void setCurveType(CurveType type) { curveType = type; }
    
    /** Gets the current modulation enabled state */
    bool isModulationEnabled() const { return modulationEnabled; }
    
    /** Gets the current expression enabled state */
    bool isExpressionEnabled() const { return expressionEnabled; }
    
    /** Gets the current curve type */
    CurveType getCurveType() const { return curveType; }
    
    /** Gets the current note velocity based on mouse Y position (127 at top, 0 at bottom) */
    int getCurrentNoteVelocity() const { return currentNoteVelocity; }
    
    /** Callback for MIDI message output */
    std::function<void(const juce::MidiMessage&)> onMidiMessage;
    
    /** Callback when X direction changes (bellows direction change) */
    std::function<void()> onDirectionChange;
    
    /** Starts global mouse tracking */
    void startTracking();
    
    /** Stops global mouse tracking */
    void stopTracking();

private:
    //==============================================================================
    // Timer callback for polling mouse position
    void timerCallback() override;
    
    //==============================================================================
    bool modulationEnabled = true;      // CC1 enabled by default
    bool expressionEnabled = true;      // CC11 enabled by default
    CurveType curveType = CurveType::Linear;
    
    int currentNoteVelocity = 0;        // Current velocity based on Y position
    
    // Direction tracking
    bool isMovingRight = true;          // Track horizontal direction
    bool wasMovingInLastFrame = false;  // Track if mouse was moving
    juce::int64 lastXMovementTime = 0;  // Time of last X movement
    static constexpr juce::int64 decayDelayMs = 100; // Time delay before CC decay starts
    static constexpr juce::int64 ccDecayDurationMs = 200; // Duration of CC value decay to 0
    
    // Velocity scaling constants
    static constexpr float maxVelocityPixelsPerSecond = 2000.0f;  // Max velocity for normalization
    
    juce::Point<int> lastMousePosition;
    juce::Point<int> currentMousePosition;
    juce::int64 lastMouseTime = 0;
    
    int lastModulationValue = 64;   // Last sent CC1 value (0-127)
    int lastExpressionValue = 64;   // Last sent CC11 value (0-127)
    
    juce::Rectangle<int> screenBounds;
    
    //==============================================================================
    /** Processes mouse movement and generates MIDI messages */
    void processMouseMovement(const juce::Point<int>& mousePos);
    
    /** Calculates velocity from mouse Y position (127 at top, 0 at bottom) */
    int calculateVelocityFromYPosition(int yPos) const;
    
    /** Calculates X movement velocity */
    float calculateXVelocity(const juce::Point<int>& from, const juce::Point<int>& to, 
                            juce::int64 timeDelta);
    
    /** Applies curve to a normalized value (0.0 to 1.0) */
    float applyCurve(float normalizedValue) const;
    
    /** Sends CC1 (Modulation Wheel) MIDI message */
    void sendModulationCC(int value);
    
    /** Sends CC11 (Expression) MIDI message */
    void sendExpressionCC(int value);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseMidiExpression)
};
