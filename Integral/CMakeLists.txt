# @file neopz/Integral/CMakeLists.txt  -- CMake file for the Integral module

target_include_directories(pz PUBLIC 
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:${NEOPZ_INCLUDE}/Integral>
                          )

file(GLOB headers *.h)
install(FILES ${headers} DESTINATION ${NEOPZ_INCLUDE}/Integral)

file(GLOB sources *.cpp)

set(pzintegral_STAT_SRCS ${headers} ${sources} PARENT_SCOPE)


