cmake_minimum_required (VERSION 3.0)

pkg_check_modules(jCanvas REQUIRED IMPORTED_TARGET jcanvas)

macro(app)
  add_executable(${ARGV0}
    ${ARGV0}.cpp
  )

  target_link_libraries(${ARGV0}
    PRIVATE
      PkgConfig::jCanvas
      Threads::Threads
  )
endmacro()

app(jchip8)
