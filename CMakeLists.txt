
cmake_minimum_required(VERSION 2.8)
project(cmake-Logger)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.67 COMPONENTS filesystem REQUIRED)


set(SRC_PATH  src/)
set(BLD_PATH  build/)
set(PROJECT_NAME AServerDemon )
set(CLIENT_NAME SClient )

#set(WANTED_LIBRARIES  -lpthread  -g )
set(WANTED_LIBRARIES  -lpthread)
set(Boost_LIBRARIES    -lboost_filesystem -lboost_system)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(SRC_LIST ${SRC_PATH}AServerDemon.cpp SClient.cpp)

add_executable(${PROJECT_NAME} ${SRC_PATH}AServerDemon.cpp)
add_executable(${CLIENT_NAME} ${SRC_PATH}SClient.cpp)

target_link_libraries(${PROJECT_NAME} ${WANTED_LIBRARIES} ${Boost_LIBRARIES})

target_link_libraries(${CLIENT_NAME} ${WANTED_LIBRARIES} ${Boost_LIBRARIES})



add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy
                          "${CMAKE_SOURCE_DIR}/${SRC_PATH}RUN.sh"  "${CMAKE_BINARY_DIR}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy
                          "${CMAKE_SOURCE_DIR}/${SRC_PATH}SLAY.sh"  "${CMAKE_BINARY_DIR}")
                          
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy
                          "${CMAKE_SOURCE_DIR}/${SRC_PATH}CLIENTS_START.sh"  "${CMAKE_BINARY_DIR}")
                          
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy
                          "${CMAKE_SOURCE_DIR}/${SRC_PATH}PROCESS_LIST.sh"  "${CMAKE_BINARY_DIR}")
                          
                          
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD

                     COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/${SRC_PATH}CLIENT_FILES"  "${CMAKE_BINARY_DIR}/CLIENT_FILES"
                      COMMENT "COPY WANTED FILES TO build-dir.")





