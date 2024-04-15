add_library(parser STATIC parser.cpp parser.util.cpp  types.cpp parser.class.cpp parser.enum.cpp parser.function.cpp )
target_link_libraries(parser PUBLIC libreflang)
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )


