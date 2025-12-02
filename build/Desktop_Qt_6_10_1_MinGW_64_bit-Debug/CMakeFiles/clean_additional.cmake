# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\qtmasic_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\qtmasic_autogen.dir\\ParseCache.txt"
  "qtmasic_autogen"
  )
endif()
