cmake_minimum_required(VERSION 3.0.0)
project(DATACOMPRESSION_PROJECT VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(benchmarks benchmarks.cpp)

set(TONGRAMS_USE_POPCNT True)
if(TONGRAMS_USE_POPCNT)
  if(UNIX)
    # Use popcount intrinsic. Available on x86-64 since SSE4.2.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTONGRAMS_USE_POPCNT")
  endif()
endif()

if(TONGRAMS_USE_PDEP)
  if(UNIX)
    # Use parallel bit deposit intrinsic.
    # Available on x86-64 since Intel’s Haswell CPUs: BMI2 (Bit Manipulation Instruction Set 2).
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTONGRAMS_USE_PDEP")
  endif()
endif()

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-braces")
  if(TONGRAMS_USE_SANITIZERS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  endif()

endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add external libraries to include path
include_directories(${PROJECT_SOURCE_DIR}/include)
