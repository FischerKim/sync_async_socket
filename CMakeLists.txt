cmake_minimum_required(VERSION 3.10)
project(winsock_projects)
#[===[
set(Boost_INCLUDE_DIR "D:/OpenSource/boost142/include/boost-1_83") 
set(Boost_LIBRARY_DIR "D:/OpenSource/boost142/lib")

set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.83.0)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
  add_executable(boost_async boost_async.cpp)
  target_link_libraries(boost_async ${Boost_LIBRARIES})
  add_executable(boost_dummy boost_dummy.cpp)
  target_link_libraries(boost_dummy ${Boost_LIBRARIES})
  #target_link_libraries(boost_async ${Boost_LIBRARY_DIR})
endif()
]===]

add_executable(async_winsock async_winsock.cpp)
add_executable(sync_winsock sync_winsock.cpp)
add_executable(dummy dummy.cpp)