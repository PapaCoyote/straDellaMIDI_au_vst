#include "MouseMidiExpression.h"

//==============================================================================
MouseMidiExpression::MouseMidiExpression()
{
    // Initialize positions
    lastMousePosition = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition().toInt();
    currentMousePosition = lastMousePosition;
    lastMouseTime = juce::Time::currentTimeMillis();
    lastXMovementTime = lastMouseTime;
    
    // Get desktop bounds for expression calculation
    screenBounds = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->totalArea;
    
    // Initialize note velocity based on starting Y position
    currentNoteVelocity = calculateVelocityFromYPosition(lastMousePosition.y);
}

MouseMidiExpression::~MouseMidiExpression()
{
    stopTracking();
}

void MouseMidiExpression::startTracking()
{
    // Start timer to poll mouse position globally
    startTimer(16); // ~60 Hz polling
}

void MouseMidiExpression::stopTracking()
{
    stopTimer();
}

void MouseMidiExpression::timerCallback()
{
    // Poll mouse position globally
    auto mousePos = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition().toInt();
    
    // Process on position change OR to handle CC decay
    if (mousePos != currentMousePosition)
    {
        processMouseMovement(mousePos);
    }
    else
    {
        // Even if mouse hasn't moved, process to handle CC decay
        juce::int64 currentTime = juce::Time::currentTimeMillis();
        juce::int64 timeSinceLastXMovement = currentTime - lastXMovementTime;
        
        // If we should be decaying, process movement with same position
        if (timeSinceLastXMovement > decayDelayMs && (lastModulationValue > 0 || lastExpressionValue > 0))
        {
            processMouseMovement(mousePos);
        }
    }
}

//==============================================================================
void MouseMidiExpression::processMouseMovement(const juce::Point<int>& mousePos)
{
    currentMousePosition = mousePos;
    juce::int64 currentTime = juce::Time::currentTimeMillis();
    juce::int64 timeDelta = currentTime - lastMouseTime;
    
    // Avoid division by zero
    if (timeDelta <= 0)
        timeDelta = 1;
    
    // Calculate note velocity from Y position (127 at top, 0 at bottom)
    currentNoteVelocity = calculateVelocityFromYPosition(mousePos.y);
    
    // Calculate X movement
    int deltaX = currentMousePosition.x - lastMousePosition.x;
    bool isMovingInX = std::abs(deltaX) > 0;
    
    // Update direction and detect changes if moving in X
    if (isMovingInX)
    {
        bool isMovingRightNow = (deltaX > 0);
        
        // Detect direction change only if we were moving in the previous frame
        if (wasMovingInLastFrame && isMovingRightNow != isMovingRight)
        {
            // Direction changed! Trigger note retrigger callback
            if (onDirectionChange)
            {
                onDirectionChange();
            }
        }
        
        // Update direction
        isMovingRight = isMovingRightNow;
        lastXMovementTime = currentTime;
        wasMovingInLastFrame = true;
    }
    else
    {
        // Not moving in X - reset the flag for accurate direction change detection
        wasMovingInLastFrame = false;
    }
    
    // Check if we should decay CC values (no X movement for decay delay time)
    juce::int64 timeSinceLastXMovement = currentTime - lastXMovementTime;
    bool shouldDecay = timeSinceLastXMovement > decayDelayMs;
    
    // Calculate CC values based on Y position, but only when moving in X
    int cc1Value = 0;
    int cc11Value = 0;
    
    if (isMovingInX)
    {
        // Use Y position for CC values (same as note velocity)
        int ccValue = currentNoteVelocity;
        
        // Apply curve
        float normalized = (float)ccValue / 127.0f;
        float curved = applyCurve(normalized);
        ccValue = (int)(curved * 127.0f);
        
        // Both CCs use same value when moving
        cc1Value = ccValue;
        cc11Value = ccValue;
    }
    else if (shouldDecay)
    {
        // Smooth decay to 0 - each CC decays from its own last value
        // Calculate remaining factor (1.0 = full value, 0.0 = fully decayed)
        float remainingFactor = 1.0f - juce::jlimit(0.0f, 1.0f, 
            (float)(timeSinceLastXMovement - decayDelayMs) / (float)ccDecayDurationMs);
        
        cc1Value = (int)(lastModulationValue * remainingFactor);
        cc11Value = (int)(lastExpressionValue * remainingFactor);
        
        // If decay is complete, ensure values are actually 0
        if (remainingFactor <= 0.0f)
        {
            cc1Value = 0;
            cc11Value = 0;
        }
    }
    else
    {
        // Keep last values briefly during delay period
        cc1Value = lastModulationValue;
        cc11Value = lastExpressionValue;
    }
    
    // Send CC1 (Modulation Wheel) if enabled
    if (modulationEnabled)
    {
        // Only send if value changed significantly
        if (std::abs(cc1Value - lastModulationValue) >= 1)
        {
            sendModulationCC(cc1Value);
            lastModulationValue = cc1Value;
        }
    }
    
    // Send CC11 (Expression) with its own value if enabled
    if (expressionEnabled)
    {
        // Only send if value changed significantly
        if (std::abs(cc11Value - lastExpressionValue) >= 1)
        {
            sendExpressionCC(cc11Value);
            lastExpressionValue = cc11Value;
        }
    }
    
    // Update tracking variables
    lastMousePosition = currentMousePosition;
    lastMouseTime = currentTime;
}

int MouseMidiExpression::calculateVelocityFromYPosition(int yPos) const
{
    // Map Y position to velocity: top of screen (y=0) = 127, bottom = 0
    int screenHeight = screenBounds.getHeight();
    
    // Guard against division by zero (edge case with unusual display configurations)
    if (screenHeight <= 0)
        screenHeight = 1;
    
    float normalizedY = (float)yPos / (float)screenHeight;
    normalizedY = juce::jlimit(0.0f, 1.0f, normalizedY);
    
    // Invert: top = 127, bottom = 0
    int velocity = (int)((1.0f - normalizedY) * 127.0f);
    return juce::jlimit(0, 127, velocity);
}

float MouseMidiExpression::calculateXVelocity(const juce::Point<int>& from, 
                                              const juce::Point<int>& to, 
                                              juce::int64 timeDelta)
{
    // Calculate X distance moved
    float dx = (float)(to.x - from.x);
    float distance = std::abs(dx);
    
    // Calculate velocity in pixels per second
    float timeInSeconds = timeDelta / 1000.0f;
    if (timeInSeconds > 0)
        return distance / timeInSeconds;
    
    return 0.0f;
}

float MouseMidiExpression::applyCurve(float normalizedValue) const
{
    normalizedValue = juce::jlimit(0.0f, 1.0f, normalizedValue);
    
    switch (curveType)
    {
        case CurveType::Linear:
            return normalizedValue;
            
        case CurveType::Exponential:
            // Exponential curve: x^2
            return normalizedValue * normalizedValue;
            
        case CurveType::Logarithmic:
            // Logarithmic curve: approximated with sqrt
            return std::sqrt(normalizedValue);
            
        default:
            return normalizedValue;
    }
}

void MouseMidiExpression::sendModulationCC(int value)
{
    value = juce::jlimit(0, 127, value);
    
    if (onMidiMessage)
    {
        // CC1 = Modulation Wheel, using channel 1 (MIDI channels are 1-based in the API)
        auto message = juce::MidiMessage::controllerEvent(1, 1, value);
        
        #if JUCE_DEBUG
        juce::Logger::writeToLog("Sending CC1 (Modulation): value=" + juce::String(value) + 
                                  " channel=" + juce::String(message.getChannel()) +
                                  " controller=" + juce::String(message.getControllerNumber()) +
                                  " controllerValue=" + juce::String(message.getControllerValue()));
        #endif
        
        onMidiMessage(message);
    }
}

void MouseMidiExpression::sendExpressionCC(int value)
{
    value = juce::jlimit(0, 127, value);
    
    if (onMidiMessage)
    {
        // CC11 = Expression, using channel 1 (MIDI channels are 1-based in the API)
        auto message = juce::MidiMessage::controllerEvent(1, 11, value);
        
        #if JUCE_DEBUG
        juce::Logger::writeToLog("Sending CC11 (Expression): value=" + juce::String(value) + 
                                  " channel=" + juce::String(message.getChannel()) +
                                  " controller=" + juce::String(message.getControllerNumber()) +
                                  " controllerValue=" + juce::String(message.getControllerValue()));
        #endif
        
        onMidiMessage(message);
    }
}
