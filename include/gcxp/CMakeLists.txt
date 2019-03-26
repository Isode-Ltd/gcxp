add_library(headers INTERFACE)
target_sources(headers INTERFACE codec.h exception.h message.h stream.h)
target_include_directories(headers INTERFACE ${PROJECT_SOURCE_DIR}/include)
