cmake_minimum_required(VERSION 3.12)
project(PG3D-Simple-Injector)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common optimizations for size and speed
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Os")

# Add compiler-specific flags
if(MSVC)
    # MSVC-specific optimizations
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O3 /Ob2 /Ot /GL /GS-")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG /OPT:REF /OPT:ICF")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Clang-specific optimizations
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Oz -flto -fno-stack-protector")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -Wl,--gc-sections -Wl,--icf=safe")
endif()

# Add source files
set(SOURCE_FILES Injector.cpp)

# Create executable
add_executable(PG3D-Simple-Injector ${SOURCE_FILES})
