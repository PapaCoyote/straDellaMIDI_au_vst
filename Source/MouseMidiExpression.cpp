#include "MouseMidiExpression.h"

//==============================================================================
MouseMidiExpression::MouseMidiExpression()
{
    // Initialize positions
    lastMousePosition = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition().toInt();
    currentMousePosition = lastMousePosition;
    lastMouseTime = juce::Time::currentTimeMillis();
    
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
    
    if (mousePos != currentMousePosition)
    {
        processMouseMovement(mousePos);
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
    
    // Calculate X movement for direction-change detection
    int deltaX = currentMousePosition.x - lastMousePosition.x;
    bool isMovingInX = std::abs(deltaX) > 0;
    
    // Detect direction changes and optionally retrigger notes
    if (isMovingInX)
    {
        bool isMovingRightNow = (deltaX > 0);
        
        if (retriggerOnDirectionChangeEnabled && wasMovingInLastFrame && isMovingRightNow != isMovingRight)
        {
            if (onDirectionChange)
                onDirectionChange();
        }
        
        isMovingRight = isMovingRightNow;
        wasMovingInLastFrame = true;
    }
    else
    {
        wasMovingInLastFrame = false;
    }
    
    // CC values always track Y position
    float normalized = (float)currentNoteVelocity / 127.0f;
    float curved = applyCurve(normalized);
    int ccValue = (int)(curved * 127.0f);
    
    // Send CC1 (Modulation Wheel) if enabled and value changed
    if (modulationEnabled && std::abs(ccValue - lastModulationValue) >= 1)
    {
        sendModulationCC(ccValue);
        lastModulationValue = ccValue;
    }
    
    // Send CC11 (Expression) if enabled and value changed
    if (expressionEnabled && std::abs(ccValue - lastExpressionValue) >= 1)
    {
        sendExpressionCC(ccValue);
        lastExpressionValue = ccValue;
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
