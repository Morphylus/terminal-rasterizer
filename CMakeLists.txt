cmake_minimum_required(VERSION 4.0.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(TerminalRasterizer  VERSION 1.0
                            DESCRIPTION "A simple terminal rasterizer"
                            LANGUAGES C
)

add_executable(rasterizer   src/main.c
                            src/core/renderer.c
                            src/core/geometry.c
                            src/core/terminal.c
                            src/core/vec.c

)

set_property(TARGET rasterizer PROPERTY C_STANDARD 23)

target_include_directories(rasterizer PUBLIC include)
