cmake_minimum_required(VERSION 3.16)
project(startrek C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# The game is 16-bit code: char is signed, and integer behaviour should not
# depend on the optimiser.
add_compile_options(-fsigned-char -fwrapv -fno-strict-aliasing -Wall -Wextra
                    -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable
                    -Wno-sign-compare -Wno-implicit-fallthrough -Wno-parentheses
                    -Wno-missing-field-initializers -Wno-pointer-sign
                    -Wno-error=incompatible-pointer-types)

file(GLOB GAME_SRC CONFIGURE_DEPENDS src/*.c src/rooms/*.c)
set(AUDIO_SRC audio/audio.c audio/audio_drv.c audio/fm.c audio/sb.c audio/mod.c audio/amiga_sfx.c
              audio/nuked/opl3.c audio/micromod/micromod.c)
add_library(game STATIC ${GAME_SRC} ${AUDIO_SRC})
target_include_directories(game PUBLIC include audio)
target_link_libraries(game PUBLIC m)

option(PLATFORM_NULL "Build the headless host instead of SDL" OFF)
if(PLATFORM_NULL)
  set(PLATFORM_SRC platform/main.c platform/plat_files.c platform/plat_null.c platform/plat_audio_null.c)
else()
  find_package(SDL2 REQUIRED)
  set(PLATFORM_SRC platform/main.c platform/plat_files.c platform/plat_sdl.c platform/plat_audio_sdl.c)
endif()
add_executable(startrek ${PLATFORM_SRC})
target_include_directories(startrek PRIVATE include audio)
target_link_libraries(startrek game)
if(NOT PLATFORM_NULL)
  target_link_libraries(startrek SDL2::SDL2)
endif()
