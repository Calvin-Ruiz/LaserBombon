cmake_minimum_required(VERSION 3.16)
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} )

project(LaserBombon)

file(GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/EntityCore/*/*.cpp"
    "${PROJECT_SOURCE_DIR}/EntityLib/*/*.cpp"
    "${PROJECT_SOURCE_DIR}/LaserBombon/*.cpp"
    )

#file(GLOB lb_SRCS "${PROJECT_SOURCE_DIR}/LaserBombon/*.cpp")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

FIND_PACKAGE(Vulkan 1.2.182 REQUIRED)
INCLUDE_DIRECTORIES(${Vulkan_INCLUDE_DIRS})

if (UNIX)
FIND_PACKAGE(SDL2 REQUIRED)
FIND_PACKAGE(SDL2_mixer REQUIRED)
FIND_PACKAGE(SDL2_ttf REQUIRED)
SET(CMAKE_CXX_FLAGS "-g3 -Wextra -Wall -Wno-sign-compare")
else()
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories($ENV{VULKAN_SDK}/Lib)
endif()

set(CPACK_SOURCE_IGNORE_FILES "/.git/" "/build/")

add_executable(${PROJECT_NAME} ${all_SRCS})
if (UNIX)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY} ${Vulkan_LIBRARIES})
else()
target_link_libraries(${PROJECT_NAME} SDL2 SDL2_ttf SDL2_mixer ${Vulkan_LIBRARIES})
file(GLOB CARRIED_DLL ${PROJECT_SOURCE_DIR}/lib/*.dll)
install(FILES ${CARRIED_DLL} DESTINATION ${CMAKE_SOURCE_DIR}/bin)
endif()

install(DIRECTORY DESTINATION ${CMAKE_SOURCE_DIR}/bin/saves)
install(DIRECTORY DESTINATION ${CMAKE_SOURCE_DIR}/bin/screenshots)
install(DIRECTORY DESTINATION ${CMAKE_SOURCE_DIR}/bin/cache)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/textures DESTINATION ${CMAKE_SOURCE_DIR}/bin/textures)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/shader/compiled DESTINATION ${CMAKE_SOURCE_DIR}/bin/shader)
install(FILES ${CMAKE_SOURCE_DIR}/resources/config.txt DESTINATION ${CMAKE_SOURCE_DIR}/bin)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/bin)
