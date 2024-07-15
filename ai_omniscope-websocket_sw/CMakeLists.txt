cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(ws LANGUAGES CXX VERSION 1.1.1)

include(FetchContent)

# Ensure position independent code is enabled for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# nlohmann_json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# asio
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio
    GIT_TAG asio-1-30-2
)
FetchContent_MakeAvailable(asio)

# Set asio include directory for Crow
set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include)

# Crow
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow
    GIT_TAG v1.1.0
)
FetchContent_MakeAvailable(Crow)

# fmt
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

# Set position independent code for fmt
set_target_properties(fmt PROPERTIES POSITION_INDEPENDENT_CODE ON
)
# Boost
find_package(Boost REQUIRED)

add_executable(${PROJECT_NAME} src/ws.cpp)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Include directories for asio (header-only)
target_include_directories(${PROJECT_NAME} PRIVATE ${asio_SOURCE_DIR}/asio/include)

# Include directories for OmniscopeSampler
target_include_directories(${PROJECT_NAME} PRIVATE /home/arkadiusz/Projects/OmniView/ai_omniscope-v2-communication_sw/src)

# Include directories for aglio
target_include_directories(${PROJECT_NAME} PRIVATE /home/arkadiusz/Projects/OmniView/aglio/src)

# Include directories for libusb
target_include_directories(${PROJECT_NAME} PRIVATE /usr/include/libusb-1.0)

# Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PUBLIC Crow::Crow)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC usb-1.0)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost)

# Asio is header-only, no need to link it

# Pthread for multi-threading support (optional, but commonly required with asio)
target_link_libraries(${PROJECT_NAME} PUBLIC pthread)

