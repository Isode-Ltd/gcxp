add_executable(producer main.cpp)
target_link_libraries(producer PRIVATE ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})
