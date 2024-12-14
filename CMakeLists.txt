cmake_minimum_required(VERSION 3.25)

project(
    "ShellProject"
    VERSION 1.0.0
    DESCRIPTION "Second lab from Operating Systems I course"
    LANGUAGES C
)

include(CTest)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic -Werror -Wunused-parameter -Wmissing-prototypes -Wstrict-prototypes")

include_directories(include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

find_package(cJSON REQUIRED)
find_package(unity REQUIRED)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/monitor/Makefile")
    message(STATUS "Cloning submodule...")
    execute_process(COMMAND git submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

execute_process(COMMAND make all
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/monitor)

add_executable(${PROJECT_NAME} src/commands.c src/monitor.c src/pipe.c src/utils.c src/config_search.c src/main.c )
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE cjson::cjson unity::unity)

if(RUN_COVERAGE EQUAL 1)
    message("Run with coverage")
endif()

add_executable(test_commands
    test/test_commands.c
    src/commands.c
    src/monitor.c
    src/pipe.c
    src/utils.c
    src/config_search.c
)
target_include_directories(test_commands PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_commands PRIVATE unity::unity cjson::cjson)
add_test(NAME test_commands COMMAND test_commands)

add_executable(test_monitor
    test/test_monitor.c
    src/commands.c
    src/monitor.c
    src/pipe.c
    src/utils.c
    src/config_search.c
)
target_include_directories(test_monitor PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_monitor PRIVATE unity::unity cjson::cjson)
add_test(NAME test_monitor COMMAND test_monitor)