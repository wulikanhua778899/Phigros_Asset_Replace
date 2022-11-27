cmake_minimum_required(VERSION 3.2)
project(Replace)

include_directories(include)
link_directories(lib)
aux_source_directory(src SOURCES_LIST)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} ${SOURCES_LIST})
target_link_libraries(${PROJECT_NAME} liblz4_64)
