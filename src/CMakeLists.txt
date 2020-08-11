cmake_minimum_required (VERSION 3.0)

include_directories(
  "${CMAKE_SOURCE_DIR}/include"
  ${JLIBCPP_INCLUDE_DIRS}
)

macro(app)
  add_executable(${ARGV0}
    ${ARGV0}.cpp
  )
target_link_libraries(${ARGV0}
    ${JLIBCPP_LIBRARIES}
  )
endmacro()

app(jchip8)
