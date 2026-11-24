cmake_minimum_required(VERSION 3.20)
project(GuardianesDePuebla LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# raylib se descarga solo, siempre la misma version para todo el equipo
include(FetchContent)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

# Toma todos los .cpp de src/: al agregar una clase nueva no hay que tocar este archivo
file(GLOB_RECURSE FUENTES CONFIGURE_DEPENDS src/*.cpp)

add_executable(guardianes ${FUENTES})
target_link_libraries(guardianes PRIVATE raylib)
target_compile_options(guardianes PRIVATE -Wall -Wextra)
