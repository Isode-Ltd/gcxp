add_executable(producer main.cpp)
target_link_libraries(producer PRIVATE Boost::thread Boost::system)
