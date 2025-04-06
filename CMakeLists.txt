cmake_minimum_required(VERSION 3.10)
project(Boltpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(Boltpp STATIC)

target_sources(Boltpp PRIVATE
  src/json.cpp
  src/httpserver.cpp
  src/response.cpp
  src/utils.cpp
)

target_include_directories(Boltpp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if(WIN32)
  target_link_libraries(Boltpp ws2_32)
endif()

if(UNIX)
  target_link_libraries(Boltpp pthread)
endif()

install(TARGETS Boltpp
  EXPORT BoltppTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

install(EXPORT BoltppTargets
  FILE BoltppConfig.cmake
  NAMESPACE Boltpp::
  DESTINATION lib/cmake/Boltpp
)