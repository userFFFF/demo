cmake_minimum_required(VERSION 3.9)
project(uac C)

set(CMAKE_C_STANDARD 99)

add_executable(uac uac.c xml_util.h xml_util.c)
target_link_libraries(uac eXosip2 osip2 osipparser2 mxml zlog)
