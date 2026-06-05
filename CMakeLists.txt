cmake_minimum_required(VERSION 3.22)
project(Granulator VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Granulator
    COMPANY_NAME                "haden"
    PLUGIN_MANUFACTURER_CODE    GLAB
    PLUGIN_CODE                 Gran
    FORMATS                     VST3 AU Standalone
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             "Instrument" "Synth"
    AU_MAIN_TYPE                kAudioUnitType_MusicDevice
    PRODUCT_NAME                "Granulator"
)

target_sources(Granulator PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
    source/grain/GrainEnvelope.cpp
    source/grain/GrainBuffer.cpp
    source/grain/GranularEngine.cpp
    source/synth/GranularVoice.cpp
    source/synth/GranularSynth.cpp
    source/modulation/LFO.cpp
    source/modulation/EnvelopeFollower.cpp
    source/modulation/ModMatrix.cpp
    source/dsp/EffectsChain.cpp
    source/ui/GranularLookAndFeel.cpp
    source/ui/components/KnobWithLabel.cpp
    source/ui/components/WaveformDisplay.cpp
    source/ui/components/FilterCurveDisplay.cpp
    source/ui/components/ReverbBlobDisplay.cpp
    source/ui/components/GrainDotDisplay.cpp
    source/ui/components/ModMatrixGrid.cpp
    source/ui/sections/GrainCoreSection.cpp
    source/ui/sections/PlayheadSection.cpp
    source/ui/sections/PitchSection.cpp
    source/ui/sections/AmplitudeSection.cpp
    source/ui/sections/SpatialSection.cpp
    source/ui/sections/ModSection.cpp
    source/ui/sections/FilterSection.cpp
    source/ui/sections/BufferSection.cpp
    source/ui/sections/EffectsSection.cpp
)

target_compile_definitions(Granulator PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_include_directories(Granulator PRIVATE source)

target_link_libraries(Granulator PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)
