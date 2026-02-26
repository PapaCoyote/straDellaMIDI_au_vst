# Mouse MIDI Expression Feature

## Overview

The Mouse MIDI Expression feature emulates the action of accordion bellows by translating mouse movement into MIDI Control Change (CC) messages. This provides expressive control over modulation and expression in real-time, while also mimicking authentic accordion behavior where notes require bellows movement to produce sound.

**Key Features:**
- **Global mouse tracking** - Works across entire desktop, not just within the application window
- **No visual area required** - Mouse tracking happens in the background
- **Real-time MIDI CC generation** - Immediate response to mouse movement
- **Authentic accordion behavior** - Notes default to 0 velocity (silent) unless bellows (mouse) is moving

## Features

### CC1 (Modulation Wheel) - Mouse Velocity
- Controlled by the speed of mouse movement
- Faster mouse movement = higher modulation value
- Range: 0-127 (MIDI standard)
- Tracks velocity in pixels per second
- Independent enable/disable control

### CC11 (Expression) - Mouse Velocity  
- Also controlled by the speed of mouse movement (same as CC1)
- Provides additional expressive control tied to bellows motion
- Range: 0-127 (MIDI standard)
- Both CC1 and CC11 sent simultaneously (no latency increase)
- Independent enable/disable control

### Key Press Velocity Control
- **Default: 0 (silent)** - Mimics accordion behavior where keys produce no sound without bellows movement
- **Adjustable: 0-127** - Slider in settings allows increasing base velocity for more expressiveness
- Configurable via "Expression Settings" window

## User Interface

### Mouse Expression Tracking
- **Global tracking** - Mouse movement is tracked across the entire desktop
- **No visual area** - No dedicated GUI component needed
- **Transparent operation** - Works in the background while you use the application

### Settings Window
Access via the "Expression Settings" button in the top-right corner.

**Available Settings:**
1. **CC1 (Modulation) Checkbox** - Enable/disable velocity-based modulation control
2. **CC11 (Expression) Checkbox** - Enable/disable velocity-based expression control
3. **Response Curve Selector** - Choose how MIDI values respond to input:
   - **Linear**: Direct 1:1 mapping (default)
   - **Exponential**: Slower response at low values, faster at high values (x²)
   - **Logarithmic**: Faster response at low values, slower at high values (√x)
4. **Key Press Velocity Slider** - Set base note velocity (0-127):
   - **0 (default)**: Silent keys without mouse movement (authentic accordion)
   - **1-127**: Adds expressiveness by allowing keys to produce sound even when mouse is still

## Usage

1. **Launch the application** and ensure MIDI output is connected
2. **Move your mouse** anywhere on your desktop to generate CC messages
3. **Play notes** using the keyboard - they will be silent unless you adjust the velocity slider
4. **Adjust settings** by clicking "Expression Settings" button:
   - Check/uncheck CC1 or CC11 as needed
   - Select desired response curve
   - Adjust Key Press Velocity slider (0 for authentic accordion behavior)
   - Close settings window

**Authentic Accordion Behavior:** By default, keys produce no sound (velocity=0) until you move the mouse (bellows). This mimics real accordion behavior where the bellows must be moving to produce sound.

## Technical Details

### MIDI CC Messages
- **CC1**: MIDI standard for Modulation Wheel
- **CC11**: MIDI standard for Expression
- **Channel**: All messages sent on MIDI Channel 1
- **Value Range**: 0-127 for both controllers
- **Latency**: Both CC messages sent in same processing cycle (no additional latency)

### Global Mouse Tracking
- Uses JUCE's Desktop API to get global mouse position
- Polls at ~60 Hz (16ms intervals) via Timer
- Works across entire primary display
- No window focus required

### Velocity Calculation
Both CC1 and CC11 use the same velocity calculation:
```
velocity = distance / time_delta
normalized_velocity = min(1.0, velocity / 2000.0)
midi_value = apply_curve(normalized_velocity) * 127
```

### Key Press Velocity
```
note_velocity = baseNoteVelocity  // Default: 0 (silent)
```
- Adjustable via slider: 0-127
- 0 = authentic accordion (no sound without bellows movement)
- Higher values = more expressiveness

### Curve Functions
- **Linear**: `f(x) = x`
- **Exponential**: `f(x) = x²`
- **Logarithmic**: `f(x) = √x`

### Performance Optimizations
- Only sends CC messages when value changes by ≥1
- Caches last sent values to avoid redundant MIDI traffic
- Uses callback architecture for non-blocking MIDI output
- Visual updates use JUCE's efficient repaint system

## Integration

### Components
- **MouseMidiExpression**: Core component handling mouse input and MIDI generation
  - Uses Timer for global mouse position polling
  - No visual component needed
  - Tracks mouse across entire desktop
- **MouseMidiSettingsWindow**: Configuration UI for user preferences
- **MainComponent**: Integrates mouse expression with existing keyboard functionality

### MIDI Output Flow
```
Global Mouse Movement → Timer Poll → Velocity Calculation → CC1 + CC11 (simultaneous) → MIDI Output Device
Key Press → baseNoteVelocity → Note On → MIDI Output Device
```

### Message Display
All CC messages and note events are logged in the MIDI Message Display panel at the bottom of the window.

## Future Enhancements

Possible additions for future versions:
- Velocity sensitivity slider
- Custom curve editor with visual preview
- CC value range limiting (min/max)
- Multiple assignable CC numbers
- Preset management for different configurations
- Mouse button modifiers for temporary effect changes
- Smoothing/filtering options for CC output

## Troubleshooting

**Mouse movements aren't generating MIDI:**
- Ensure at least one CC type is enabled in settings
- Check MIDI output device is connected
- Verify application is running (tracking works even when window is not focused)
- Check console logs for debug messages

**CC values seem wrong or inverted:**
- Try different curve types in settings
- Both CC1 and CC11 use the same velocity value (from mouse speed)
- Check that your DAW is receiving on MIDI Channel 1

**Keys aren't making sound:**
- This is expected! By default, key press velocity is 0 (authentic accordion behavior)
- Increase "Key Press Velocity" slider in Expression Settings
- Or ensure you're moving the mouse (bellows) while playing

**DAW not receiving CC messages:**
- Verify the application's MIDI output is connected to your DAW input
- Check your DAW's MIDI learn or monitor to see incoming messages
- Some DAWs require you to enable CC input or map controllers
- Check console logs to verify messages are being sent with correct format

## Code Files

- `Source/MouseMidiExpression.h` - Header for mouse MIDI component
- `Source/MouseMidiExpression.cpp` - Implementation of mouse MIDI logic
- `Source/MouseMidiSettingsWindow.h` - Header for settings UI
- `Source/MouseMidiSettingsWindow.cpp` - Implementation of settings UI
- `Source/MainComponent.h` - Updated to integrate mouse expression
- `Source/MainComponent.cpp` - Updated to wire mouse expression to MIDI output

## Building

After adding these files, rebuild the project using Projucer:
1. Open `straDellaMIDI.jucer` in Projucer
2. Click "Save Project" to regenerate build files
3. Build using your platform's IDE (Xcode, Visual Studio, etc.)

The new source files are already included in the .jucer project file.
