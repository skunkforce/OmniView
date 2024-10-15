cmake_minimum_required(VERSION 3.20)

# Searching for VCPKG-installation and download if needed
include(FetchContent)
FetchContent_Populate(
  vcpkg
  GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
  GIT_TAG        2023.11.20
  SOURCE_DIR     "${CMAKE_SOURCE_DIR}/vcpkg"
)

# saves vcpkg build artifacts in vcpkg directory
list(APPEND CMAKE_PROJECT_TOP_LEVEL_INCLUDES "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES CMAKE_PROJECT_TOP_LEVEL_INCLUDES)

set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")

project("OmniscopeGui" VERSION 0.9.0)

if(MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
    )
endif()


include(cmake_helpers/BuildOptions.cmake)
include(cmake_helpers/FindOrFetch.cmake)

set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(OmniView 
               src/main.cpp 
               src/generateTrainingData.cpp 
               src/info_popup.cpp
               src/imgui_stdlib.cpp
               src/saves_popup.cpp     
               src/style.cpp
               src/handler.cpp
               src/analyze_data.cpp
               ${IMGUI_SOURCES}
               )

target_add_default_build_options(OmniView PUBLIC)

find_or_fetch_package(
  cmake_git_version 0.1 GIT_REPOSITORY
  https://github.com/dominicpoeschko/cmake_git_version.git GIT_TAG master)

target_add_version_headers(OmniView ${CMAKE_BUILD_TYPE})

add_subdirectory(ImGuiInstance)
target_link_libraries(OmniView PUBLIC imgui_instance::imgui_instance)

add_subdirectory(aglio)
target_link_libraries(OmniView PUBLIC aglio::aglio)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(OmniView PUBLIC fmt::fmt-header-only)

find_package(Boost REQUIRED)
target_link_libraries(OmniView PUBLIC Boost::boost)

find_package(CURL REQUIRED)
target_link_libraries(OmniView PUBLIC ${CURL_LIBRARY} CURL::libcurl)

find_package(libusb CONFIG REQUIRED)
target_include_directories(OmniView PUBLIC ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(OmniView PUBLIC ${LIBUSB_LIBRARIES})

target_compile_definitions(OmniView PUBLIC BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT)
target_compile_features(OmniView PUBLIC cxx_std_23)

