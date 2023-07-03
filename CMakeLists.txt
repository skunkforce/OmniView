cmake_minimum_required(VERSION 3.20)

project("OmniscopeGui")

include(cmake_helpers/BuildOptions.cmake)

add_executable(OmniscopeGui src/main.cpp)

target_add_default_build_options(OmniscopeGui PUBLIC)

add_subdirectory(ImGuiInstance)
target_link_libraries(OmniscopeGui PUBLIC imgui_instance::imgui_instance)

add_subdirectory(aglio)
target_link_libraries(OmniscopeGui PUBLIC aglio::aglio)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(OmniscopeGui PUBLIC fmt::fmt)

find_package(Boost REQUIRED)
target_link_libraries(OmniscopeGui PUBLIC Boost::boost)
