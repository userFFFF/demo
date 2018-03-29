cmake_minimum_required(VERSION 3.9)
project(uac)

set(CMAKE_CXX_STANDARD 11)

include_directories(/usr/include/groupsock)
include_directories(/usr/include/UsageEnvironment)
include_directories(/usr/include/BasicUsageEnvironment)
include_directories(/usr/include/liveMedia)

add_executable(uac uac.c xml_util.h xml_util.c PsMuxExample.cpp libpsmux/inc/Gb28181PsMux.h libpsmux/inc/psmuxdef.h libpsmux/inc/PsMuxStreamType.h libpsmux/src/bits.h libpsmux/src/crc.h libpsmux/src/Gb28181PsMux.cpp libpsmux/src/psmux.cpp libpsmux/src/psmux.h libpsmux/src/psmuxcommon.h libpsmux/src/psmuxstream.cpp libpsmux/src/psmuxstream.h PsMuxExample.h Live555Example.cpp Live555Example.h)
target_link_libraries(uac eXosip2 osip2 osipparser2 jrtp mxml zlog rt BasicUsageEnvironment UsageEnvironment groupsock liveMedia)
