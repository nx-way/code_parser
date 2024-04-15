add_executable(serialize_gen serialize_gen.cpp)
target_link_libraries(serialize_gen PUBLIC parser tinyalloc)
target_include_directories(serialize_gen PUBLIC ${CMAKE_SOURCE_DIR}/include)

