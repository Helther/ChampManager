set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 5.14 COMPONENTS Core REQUIRED)
add_executable(ChampManager
  main.cpp
  parser.cpp
  )

target_compile_features(ChampManager PRIVATE cxx_std_17)
set_project_warnings(ChampManager)

target_link_libraries(ChampManager Qt5::Core)
