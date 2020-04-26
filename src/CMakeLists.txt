set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Core REQUIRED)
add_executable(ChampManager
  main.cpp
  parser.cpp
  )
set_project_warnings(ChampManager)
target_link_libraries(ChampManager Qt5::Core)
