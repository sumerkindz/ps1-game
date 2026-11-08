cmake_minimum_required(VERSION 3.21)

project(
    PS1Game
    LANGUAGES C CXX ASM
    VERSION 0.1.0
    DESCRIPTION "A first PlayStation 1 homebrew game"
)

psn00bsdk_add_executable(ps1game GPREL main.c)

psn00bsdk_add_cd_image(
    disc
    ps1game
    iso.xml
    DEPENDS ps1game system.cnf
)
