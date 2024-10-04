cmake_minimum_required(VERSION 3.12)

project(smartmap VERSION 0.1.0)

option(PLUGREGIE "c moi" OFF)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
# find_package(Freetype REQUIRED)

add_executable( smartmap

# src/log.cpp

main.cpp

# src/test.cpp

# src/folder.cpp
# src/file.cpp
# src/json.cpp
# src/image.cpp



)
target_compile_features(smartmap PUBLIC cxx_std_23)
set_target_properties(smartmap PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(
  smartmap PROPERTIES COMPILE_FLAGS
                      "-DIMGUI_IMPL_OPENGL_LOADER_GL3W -Xlinker --verbose ")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

file(COPY assets DESTINATION .)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
# include_directories(${})
include_directories(${CMAKE_SOURCE_DIR}/vendors)
add_definitions(-DFT2_BUILD_LIBRARY)
target_include_directories(
  smartmap
  PRIVATE .
          src
          vendors/imgui/
          vendors/gl3w/include/
          vendors/glm/
          vendors/ft/include/
          # ${FREETYPE_INCLUDE_DIRS}

)

# link_directories(${FREETYPE_LIBRARY_DIRS})

target_link_libraries(
  smartmap PRIVATE OpenGL::GL glfw assimp # ${FREETYPE_LIBRARIES}
)

target_link_directories(smartmap PRIVATE
                        ${CMAKE_CURRENT_SOURCE_DIR}/vendors/NDI/Lib/
                        vendors/
                        )

if(WIN32)
  file(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/vendors/NDI/Lib/Processing.NDI.Lib.x64.dll
    DESTINATION .)
  target_include_directories(smartmap PRIVATE vendors/NDI/Include/)
  target_link_libraries(smartmap PRIVATE ws2_32 iphlpapi Processing.NDI.Lib.x64)
  add_compile_definitions(WIN32="1")
endif(WIN32)

if(UNIX)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vendors/NDI/Lib/libndi.so DESTINATION .)
  file(CREATE_LINK libndi.so ${CMAKE_CURRENT_BINARY_DIR}/libndi.so.4
       COPY_ON_ERROR SYMBOLIC)
  target_link_libraries(smartmap PRIVATE -ldl -lndi)
  # for libartnet to use a less deprecated network api on linux
  set_target_properties(
    smartmap PROPERTIES COMPILE_FLAGS
                        "-DHAVE_GETIFADDRS -DHAVE_LINUX_IF_PACKET_H")
    target_compile_definitions(smartmap PRIVATE ISUNIX=1)
endif(UNIX)


target_compile_definitions(smartmap PRIVATE ROCH=1)
if(PLUGREGIE)
  target_compile_definitions(smartmap PRIVATE PLUGREGIE=1)
endif(PLUGREGIE)
