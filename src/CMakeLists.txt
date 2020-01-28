cmake_minimum_required(VERSION 3.10)

# set the project name
project(DSS_Assignment)

# add other paths
include_directories(${CMAKE_SOURCE_DIR}/inc)
include_directories(${CMAKE_SOURCE_DIR}/lib)

#### link_directories(${CMAKE_SOURCE_DIR}/lib)

#### add_library(libcurl SHARED IMPORTED)
#### set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libcurl.a")
#### add_library(libcurldll SHARED IMPORTED)
#### set_property(TARGET libcurldll PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libcurl.dll.a")

#### add_library(libcurl SHARED IMPORTED)
#### TARGET_LINK_LIBRARIES(TARGET_FILE "-L/path/to/shared/library" SHARED_LIB_name)
#### set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libcurl-x64.dll")

# add the executable
add_executable(main main.c)

# add link libraries
#### target_link_libraries(main ${CMAKE_SOURCE_DIR}/lib/libcurl.a)

ADD_LIBRARY(libcurl STATIC IMPORTED)
SET_TARGET_PROPERTIES(libcurl PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libcurl.a)
TARGET_LINK_LIBRARIES(main libcurl)
