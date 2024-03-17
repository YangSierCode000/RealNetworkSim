cmake_minimum_required(VERSION 3.12)
project(RealSim)

# Add PyBind11
add_subdirectory(lib/pybind11)

# Include directories
include_directories(include/)

# Add the source files for your new application
set(SOURCE_FILES
  cpp_src/main.cpp
  cpp_src/my_application.cpp
  cpp_src/working_application.cpp  # Add your working_application source file here
)

# Link the necessary libraries
set(LIBRARIES_TO_LINK
  pybind11::embed
  ${libcore}
  ${libpoint-to-point}
  ${libinternet}
  ${libapplications}
  ${libflow-monitor}
)


# Create the executable
build_exec(
  EXECNAME MyExecutable
  SOURCE_FILES ${SOURCE_FILES}
  LIBRARIES_TO_LINK ${LIBRARIES_TO_LINK}
  EXECUTABLE_DIRECTORY_PATH ${CMAKE_OUTPUT_DIRECTORY}/application
)
