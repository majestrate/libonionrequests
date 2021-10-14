
enable_testing()

add_executable(onionrequest onionrequest.cpp)
target_link_libraries(onionrequest PUBLIC onionreq)
