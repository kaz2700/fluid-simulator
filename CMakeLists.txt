cmake_minimum_required(VERSION 3.15)
project(SPH2Simulator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(GLM REQUIRED glm)
pkg_check_modules(EPOXY REQUIRED epoxy)

include_directories(
  ${GLFW_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${EPOXY_INCLUDE_DIRS}
  src
  shaders
)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

add_executable(sph2_sim ${SOURCES} ${HEADERS})

add_custom_command(TARGET sph2_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:sph2_sim>/shaders
)

target_link_libraries(sph2_sim
  Threads::Threads
  ${GLFW_LIBRARIES}
  ${EPOXY_LIBRARIES}
)

set_target_properties(sph2_sim PROPERTIES
  CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2"
)
