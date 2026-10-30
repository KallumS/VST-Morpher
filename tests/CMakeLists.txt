# A tiny VST3 the tests load into Morpher's corners: it scales the sound by a
# gain and delays it by a latency, both set through its saved state.
juce_add_plugin(MorpherTestGain
    PRODUCT_NAME "Morpher Test Gain"
    COMPANY_NAME "Morpher Tests"
    BUNDLE_ID "com.kallumshah.morpher.testgain"
    PLUGIN_MANUFACTURER_CODE Ksha
    PLUGIN_CODE Mtg1
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(MorpherTestGain PRIVATE fixture/TestGain.cpp)
target_compile_definitions(MorpherTestGain PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(MorpherTestGain
    PRIVATE juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

juce_add_console_app(MorpherTests PRODUCT_NAME "Morpher Tests")
target_sources(MorpherTests PRIVATE
    TestMain.cpp
    MorphMathTests.cpp
    ThemeTests.cpp
    ProcessorTests.cpp
    EditorTests.cpp
    Screenshots.cpp)
target_compile_definitions(MorpherTests PRIVATE
    JUCE_STANDALONE_APPLICATION=1
    JUCE_MODAL_LOOPS_PERMITTED=1
    MORPHER_TEST_PLUGIN="$<GENEX_EVAL:$<TARGET_PROPERTY:MorpherTestGain_VST3,JUCE_PLUGIN_ARTEFACT_FILE>>")
target_link_libraries(MorpherTests
    PRIVATE morpher_core juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
add_dependencies(MorpherTests MorpherTestGain_VST3)

add_test(NAME MorpherTests COMMAND MorpherTests)
