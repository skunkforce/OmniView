# my_toolchain.cmake

# Setze das Präprozessor-Makro ImDrawIdx auf unsigned int für C++-Dateien
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DImDrawIdx=\\\"unsigned int\\\"")

# Option: Falls du vcpkg benutzt, binde die vcpkg-Toolchain-Datei ein
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    include("${CMAKE_CURRENT_LIST_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()
