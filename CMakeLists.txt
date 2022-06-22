cmake_minimum_required(VERSION 3.18)

set(PROJECT_VERSION 1.0.0)
project(CoroutineExample VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(${PROJECT_NAME} CoroutineExample.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)

include(FetchContent)
FetchContent_Declare(
    cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG ee7f91f5566b6279aadf6d0b1aaf56b11963f527
) # The commit hash for 1.8.2. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)
target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)

# Show actual C++ version for MSVC
# Use compliant C11 preprocessor for MSVC
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /Zc:__cplusplus /Zc:preprocessor)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ML99_ALLOW_POOR_DIAGNOSTICS)
# Metalang99 relies on heavy macro machinery. To avoid useless macro expansion
# errors, please write this:
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -fmacro-backtrace-limit=1)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -ftrack-macro-expansion=0)
endif()

FetchContent_Declare(
    ConstraintType
    URL https://github.com/yonghenghuanmie/Component/archive/refs/tags/v1.3.2.tar.gz # v1.3.2
    SOURCE_SUBDIR ConstraintType
)
FetchContent_MakeAvailable(ConstraintType)
target_link_libraries(${PROJECT_NAME} PRIVATE ConstraintType)