cmake_minimum_required(VERSION 3.30)
project(attaques_dofus C)

set(CMAKE_C_STANDARD 23)

add_executable(attaques_dofus main.c
        attaques.h
        attaques.c
        CLASSE_ataque.h)

target_link_libraries(attaques_dofus -lmingw32 -lalleg44)