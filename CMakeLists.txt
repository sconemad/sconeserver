cmake_minimum_required(VERSION 2.8)
project(sconeserver)
include (sconeserver.cmake)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_definitions(-DHAVE_CONFIG_H)

add_compile_options(-Wall)

# Enable ASAN
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")

add_executable(sconed main.cpp)
target_include_directories(sconed PRIVATE . .. ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(sconed sconex)

install(TARGETS sconed DESTINATION ${BIN_PATH})
install(FILES sconectl DESTINATION ${BIN_PATH} 
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
  GROUP_EXECUTE WORLD_EXECUTE)
install(FILES sconeserver.conf DESTINATION ${CONF_PATH})
install(FILES sconed.1 sconectl.1 DESTINATION ${MAN_PATH}/man1)

add_subdirectory(sconex)

add_subdirectory(bluetooth)
add_subdirectory(exec)
add_subdirectory(forward)
add_subdirectory(http)
add_subdirectory(image)
add_subdirectory(ip)
add_subdirectory(ip6)
add_subdirectory(local)
add_subdirectory(location)
add_subdirectory(markdown)
#add_subdirectory(maths)
add_subdirectory(mime)
add_subdirectory(mysql)
add_subdirectory(rss)
add_subdirectory(sconesite)
add_subdirectory(server)
add_subdirectory(simple)
add_subdirectory(smtp)
add_subdirectory(sqlite)
add_subdirectory(ssl)
add_subdirectory(stat)
add_subdirectory(test)
add_subdirectory(testbuilder)
add_subdirectory(tftp)

add_subdirectory(examples/tuesdayonly)
add_subdirectory(examples/rot13)
