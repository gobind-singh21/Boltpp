cmake_minimum_required(VERSION 3.15)
project(Boltpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find pthreads
find_package(Threads REQUIRED)

# Include headers
include_directories(include)

# Create Boltpp static library
add_library(Boltpp STATIC
    src/httpserver.cpp
    src/response.cpp
    src/utils.cpp
    src/json.cpp
    # add other source files as needed
)

if(WIN32)
  target_link_libraries(Boltpp PRIVATE ws2_32)
endif()

target_link_libraries(Boltpp PRIVATE Threads::Threads)

# Install the library
install(TARGETS Boltpp
    ARCHIVE DESTINATION lib
)

# Install public headers
install(DIRECTORY include/ DESTINATION include)
