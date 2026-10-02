cmake_minimum_required(VERSION 3.16)
project(LiveDashboard LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This project requires Linux SocketCAN and Qt5 Widgets.")
endif()
find_package(Threads REQUIRED)

find_package(Qt5 REQUIRED COMPONENTS Widgets Svg)
set(CMAKE_AUTOMOC ON)

# All binaries (engine_ecu, transmission_ecu, bcm, dashboard) land together in
# one flat build/ directory, so `./engine_ecu`, `./dashboard`, etc. all work
# from wherever you ran `make` â€” matching the commands used throughout lab.md.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(dashboard
    src/main.cpp
    src/MainWindow.cpp
    src/SpeedometerWidget.cpp
    src/RPMGaugeWidget.cpp
    src/CheckEngineIcon.cpp
    src/CANWorker.cpp
)

target_link_libraries(dashboard Qt5::Widgets Qt5::Svg)

# The Engine ECU, Transmission ECU, and BCM from Sessions 07-08, bundled here
# so this lab is fully self-contained â€” no other session's directory needs to
# exist alongside it. See lab_assets/README.md "Before you run it".
add_subdirectory(ecus/engine_ecu)
add_subdirectory(ecus/transmission_ecu)
add_subdirectory(ecus/body_control_module)
foreach(ecu engine_ecu transmission_ecu bcm)
    target_link_libraries(${ecu} PRIVATE Threads::Threads)
endforeach()

# `cmake --build . --target run_demo` (after the normal cmake/make build
# above) starts Engine ECU (--demo mode, so RPM/speed keep moving instead of
# settling on a fixed value before your dashboard even connects), Transmission
# ECU, and BCM, then launches your dashboard â€” an optional shortcut for the
# four-terminal manual startup described in README.md.
add_custom_target(run_demo
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/run_demo.sh
        $<TARGET_FILE:engine_ecu>
        $<TARGET_FILE:transmission_ecu>
        $<TARGET_FILE:bcm>
        $<TARGET_FILE:dashboard>
    DEPENDS engine_ecu transmission_ecu bcm dashboard
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    USES_TERMINAL
    VERBATIM
)
