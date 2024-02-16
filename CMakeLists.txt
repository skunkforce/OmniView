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

project("OmniscopeGui" VERSION 0.5.1)

if(MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
    )
endif()


include(cmake_helpers/BuildOptions.cmake)
include(cmake_helpers/FindOrFetch.cmake)


add_executable(OmniView src/main.cpp
src/menuscpp/Style.cpp)

target_add_default_build_options(OmniView PUBLIC)

find_or_fetch_package(
  cmake_git_version 0.1 GIT_REPOSITORY
  https://github.com/dominicpoeschko/cmake_git_version.git GIT_TAG master)

target_add_version_headers(OmniView ${CMAKE_BUILD_TYPE})

add_subdirectory(ImGuiInstance)
target_link_libraries(OmniView PUBLIC imgui_instance::imgui_instance)

add_subdirectory(aglio)
target_link_libraries(OmniView PUBLIC aglio::aglio)

#add_subdirectory(stb_Image)
#target_link_libraries(OmniView PUBLIC stb_Image::stb_Image)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(OmniView PUBLIC fmt::fmt-header-only)

find_package(Boost REQUIRED)
target_link_libraries(OmniView PUBLIC Boost::boost)

find_package(CURL REQUIRED)
target_link_libraries(OmniView PUBLIC ${CURL_LIBRARY} CURL::libcurl)

set(IMGUI_FILEBROWSER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui-filebrowser)

add_library(imgui-filebrowser INTERFACE)
target_include_directories(imgui-filebrowser INTERFACE ${IMGUI_FILEBROWSER_INCLUDE_DIR})

target_include_directories(OmniView PRIVATE src/menushpp/Style.hpp)


#target_include_directories(OmniView PRIVATE include stb_Image)

target_link_libraries(OmniView PRIVATE ${CMAKE_SOURCE_DIR}/stb_image)

find_package(libusb CONFIG REQUIRED)
target_include_directories(OmniView PUBLIC ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(OmniView PUBLIC ${LIBUSB_LIBRARIES})

target_link_libraries(OmniView PUBLIC imgui-filebrowser)

target_compile_definitions(OmniView PUBLIC BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT)
target_compile_features(OmniView PUBLIC cxx_std_20)
