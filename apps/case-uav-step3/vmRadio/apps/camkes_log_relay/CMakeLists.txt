cmake_minimum_required(VERSION 3.8.2)

project(camkes_log_relay C)
set(CMAKE_C_STANDARD 11)

add_subdirectory(${CAMKES_LOG_QUEUE_LIB} camkes_log_queue)
add_executable(camkes_log_relay camkes_log_relay.c)
target_link_libraries(camkes_log_relay camkes_log_queue -static-libgcc -static)
