# SAM is not MIT licensed: see THIRD_PARTY.md before redistribution.
create_kastle2_app(
    APP_NAME "talking-synth"
    APP_USB_NAME "Kastle 2 Talking Synth"
    APP_USB_PREFIX "K2TS_"
    APP_NAME_WITH_USER_DATA "talking-synth-with-presets"
    APP_USER_DATA ${CMAKE_CURRENT_SOURCE_DIR}/PRESETS.bin
    APP_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/AppTalkingSynth.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sam/sam.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sam/render.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sam/reciter.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sam/debug.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sam/stream.c
)
# Keep legacy SAM diagnostics quiet without applying Kastle C++ warnings to C.
