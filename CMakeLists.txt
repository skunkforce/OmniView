cmake_minimum_required(VERSION 3.20)

project("OmniscopeGui")

include(cmake_helpers/BuildOptions.cmake)

add_executable(OmniView src/main.cpp)

target_add_default_build_options(OmniView PUBLIC)

add_subdirectory(ImGuiInstance)
target_link_libraries(OmniView PUBLIC imgui_instance::imgui_instance)

add_subdirectory(aglio)
target_link_libraries(OmniView PUBLIC aglio::aglio)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(OmniView PUBLIC fmt::fmt)

find_package(Boost REQUIRED)
target_link_libraries(OmniView PUBLIC Boost::boost)

find_package(CURL REQUIRED)
target_link_libraries(OmniView PUBLIC curl)

set(IMGUI_FILEBROWSER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui-filebrowser)

add_library(imgui-filebrowser INTERFACE)
target_include_directories(imgui-filebrowser INTERFACE ${IMGUI_FILEBROWSER_INCLUDE_DIR})

target_link_libraries(OmniView PUBLIC imgui-filebrowser)


