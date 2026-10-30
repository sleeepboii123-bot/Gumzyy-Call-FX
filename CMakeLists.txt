cmake_minimum_required(VERSION 3.24)
project(GUMZYY_CALL_FX VERSION 0.3.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(GumzyyBinaryData SOURCES Assets/gumzyy.png Assets/phone_banner.png)

juce_add_plugin(GUMZYY_CALL_FX
    COMPANY_NAME "NYXV x OFF SESSION"
    BUNDLE_ID "com.nyxv.offsession.gumzyycallfx"
    PLUGIN_MANUFACTURER_CODE Nyxv
    PLUGIN_CODE GCFX
    FORMATS VST3
    PRODUCT_NAME "GUMZYY CALL FX"
    DESCRIPTION "GUMZYY ad-lib character and call FX processor - zero added plugin latency"
    VST3_CATEGORIES Fx
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(GUMZYY_CALL_FX)

target_sources(GUMZYY_CALL_FX PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/CallFxEngine.cpp
    Source/CallFxEngine.h
)

target_compile_definitions(GUMZYY_CALL_FX PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(GUMZYY_CALL_FX PRIVATE
    GumzyyBinaryData
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

include(CTest)
if(BUILD_TESTING)
    add_executable(GUMZYY_CALL_FX_DSP_TESTS
        Source/CallFxEngine.cpp
        Tests/CallFxEngineTests.cpp
    )
    target_include_directories(GUMZYY_CALL_FX_DSP_TESTS PRIVATE Source)
    target_compile_features(GUMZYY_CALL_FX_DSP_TESTS PRIVATE cxx_std_17)
    if(MSVC)
        target_compile_options(GUMZYY_CALL_FX_DSP_TESTS PRIVATE /W4 /permissive-)
    else()
        target_compile_options(GUMZYY_CALL_FX_DSP_TESTS PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME GumzyyCallFxLowLatencyDSP COMMAND GUMZYY_CALL_FX_DSP_TESTS)
endif()
