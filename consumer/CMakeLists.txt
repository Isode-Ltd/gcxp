add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})
