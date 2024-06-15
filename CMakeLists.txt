cmake_minimum_required(VERSION 3.0)

project(droppox_project)

set(CMAKE_C_STANDARD 99)

link_libraries(crypto)

add_executable(droppox_serv droppox_serv.c
        dp_lib/dp_lib_serv.c
        dp_lib/dp_lib_common.c
)

add_executable(droppox_clnt droppox_clnt.c
        dp_lib/dp_lib_clnt.c
        dp_lib/dp_lib_common.c
)