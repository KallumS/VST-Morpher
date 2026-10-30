cmake_minimum_required(VERSION 3.22)

project(Morpher VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Oldest macOS the plug-in runs on")

option(MORPHER_BUILD_TESTS "Build the test suite and its test plug-in" ON)

# JUCE and clap-juce-extensions are fetched at fixed versions. To use copies
# already on disk, pass -DFETCHCONTENT_SOURCE_DIR_JUCE=... and
# -DFETCHCONTENT_SOURCE_DIR_CLAP_JUCE_EXTENSIONS=...
include(FetchContent)

FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.4
    GIT_SHALLOW    TRUE)

FetchContent_Declare(clap_juce_extensions
    GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
    GIT_TAG        7adee3a1bd4684d4caa5601100e364abccff4b4d
    GIT_SUBMODULES_RECURSE TRUE)

FetchContent_MakeAvailable(juce)
FetchContent_GetProperties(clap_juce_extensions)
if(NOT clap_juce_extensions_POPULATED)
    FetchContent_Populate(clap_juce_extensions)
    add_subdirectory(${clap_juce_extensions_SOURCE_DIR} ${clap_juce_extensions_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(MORPHER_FORMATS VST3)
if(APPLE)
    list(APPEND MORPHER_FORMATS AU)
endif()

juce_add_plugin(Morpher
    PRODUCT_NAME "Morpher"
    COMPANY_NAME "Kallum Shah"
    BUNDLE_ID "com.kallumshah.morpher"
    PLUGIN_MANUFACTURER_CODE Ksha
    PLUGIN_CODE Mrph
    FORMATS ${MORPHER_FORMATS}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES Fx Tools
    AU_MAIN_TYPE kAudioUnitType_MusicEffect
    COPY_PLUGIN_AFTER_BUILD FALSE)

clap_juce_extensions_plugin(TARGET Morpher
    CLAP_ID "com.kallumshah.morpher"
    CLAP_FEATURES audio-effect mixing utility)

# The code shared by the plug-in and the tests.
add_library(morpher_core INTERFACE)
target_sources(morpher_core INTERFACE
    src/MorpherProcessor.cpp
    src/Slot.cpp
    src/Settings.cpp
    src/ui/MorpherEditor.cpp
    src/ui/HouseLookAndFeel.cpp
    src/ui/XYPad.cpp
    src/ui/SlotPanel.cpp
    src/ui/MessageBar.cpp
    src/ui/PluginWindow.cpp)
target_include_directories(morpher_core INTERFACE src)
target_compile_definitions(morpher_core INTERFACE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_AU=0
    JUCE_PLUGINHOST_LV2=0
    JUCE_PLUGINHOST_LADSPA=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(morpher_core INTERFACE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra)

target_sources(Morpher PRIVATE src/PluginEntry.cpp)
target_link_libraries(Morpher
    PRIVATE
        morpher_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(MORPHER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
