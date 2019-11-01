cmake_minimum_required(VERSION 3.9)

project(turn_combat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  add_compile_options("-frelaxed-template-template-args")
endif()

set(SOURCES main.cpp prisoner.cpp)

function(do_compile TARGET_NAME PGO_TRAIN)
  add_executable(${TARGET_NAME} ${SOURCES})

  target_link_libraries(${TARGET_NAME} PUBLIC ${NCURSES_LIBRARIES})
  target_include_directories(${TARGET_NAME} PUBLIC ${NCURSES_INCLUDE_DIRS})

  include(CheckIPOSupported)
  check_ipo_supported(RESULT result OUTPUT output)
  if(result)
    set_property(TARGET ${TARGET_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(WARNING "IPO is not supported: ${output}")
  endif()

  if(PGO_TRAIN)
    target_compile_options(${TARGET_NAME} PRIVATE -fprofile-generate=pgo)
    target_link_libraries(${TARGET_NAME} PRIVATE gcov)
  endif()
endfunction()

do_compile(${PROJECT_NAME} FALSE)
