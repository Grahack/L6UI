cmake_minimum_required(VERSION 3.15)

project(L6UI VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

# Remplace par le chemin vers JUCE
set(JUCE_DIR ../JUCE)

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_gui_app(L6UI
    COMPANY_NAME "ProfGra"
)

juce_generate_juce_header(L6UI)

target_sources(L6UI PRIVATE
    Main.cpp
    MainComponent.cpp
    MainComponent.h
    CustomLookAndFeel.h
)

target_compile_definitions(L6UI PRIVATE
    JUCE_APPLICATION_NAME_STRING="L6UI"
    JUCE_APPLICATION_VERSION_STRING="0.1.0"
)

target_link_libraries(L6UI PRIVATE
    juce::juce_gui_basics
    juce::juce_audio_devices
    curl
)

