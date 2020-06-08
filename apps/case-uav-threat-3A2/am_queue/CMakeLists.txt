
cmake_minimum_required(VERSION 3.7.2)

project(am_queue C)

add_library(am_queue EXCLUDE_FROM_ALL src/am_queue.c)

# Assume that if the muslc target exists then this project is in an seL4 native
# component build environment, otherwise it is in a linux userlevel environment.
# In the linux userlevel environment, the C library will be linked automatically.
if(TARGET muslc)
	target_link_libraries(am_queue muslc)
endif()

target_include_directories(am_queue PUBLIC include)
