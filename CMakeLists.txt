cmake_minimum_required(VERSION 3.15)
project(Boltpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Threads REQUIRED)

include_directories(include)

add_library(Boltpp STATIC
    src/httpserver.cpp
    src/response.cpp
    src/utils.cpp
    src/json.cpp
)

if(WIN32)
  target_link_libraries(Boltpp PRIVATE ws2_32)
endif()

target_link_libraries(Boltpp 
    PRIVATE 
        Threads::Threads 
    PUBLIC 
        stdc++fs
)

install(TARGETS Boltpp
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)
