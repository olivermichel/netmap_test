cmake_minimum_required(VERSION 3.5)
project(netmap_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-fpermissive")

find_path(NETMAP_INCLUDE NAMES net/netmap_user.h HINTS /usr/ /usr/local/)
find_file(NETMAP_HEADER NAMES net/netmap_user.h HINTS ${NETMAP_INCLUDE})
message(STATUS "netmap: ${NETMAP_HEADER}")

add_executable(receive receive.cc netmap_iface.h)

add_executable(send send.cc netmap_iface.h)
