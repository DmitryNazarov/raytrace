cmake_minimum_required(VERSION 3.5.1)

project(RayTracer VERSION 1.0.0)

set(GLM_INCLUDE_DIR "glm")

IF (WIN32)
  set(SFML_DIR "SFML-2.5.1/lib/cmake/SFML")
ENDIF()

IF (WIN32)
  find_package(SFML COMPONENTS main graphics system REQUIRED)
ELSE()
  find_package(SFML COMPONENTS graphics system window REQUIRED)
ENDIF()

find_package(Threads)

set(SOURCE
  transform.cpp
  main.cpp
  threadpool.cpp
  intersections.cpp
)

#Raytracer target
add_executable(${PROJECT_NAME} WIN32 ${SOURCE})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${GLM_INCLUDE_DIR})

IF (WIN32)
  target_link_libraries(${PROJECT_NAME} PUBLIC sfml-main sfml-graphics sfml-system)
ELSE()
  target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads sfml-graphics sfml-system sfml-window)
ENDIF()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

#Raytracer tests target
enable_testing()
IF (WIN32)
  set(gtest_force_shared_crt ON)
ENDIF()
add_subdirectory(googletest)

set(TEST_SOURCES
  tests/test_common.cpp
  tests/transform_test.cpp
  tests/triangle_test.cpp
  tests/sphere_test.cpp
  transform.cpp
  intersections.cpp
)

add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})
target_include_directories(${PROJECT_NAME}_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include ${GLM_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}_test gtest gtest_main)