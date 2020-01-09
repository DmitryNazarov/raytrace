cmake_minimum_required(VERSION 3.5.1)

project(RayTracer)

set(GLM_INCLUDE_DIR "glm")

IF (WIN32)
  set(SFML_DIR "SFML-2.5.1/lib/cmake/SFML")
ENDIF()

IF (WIN32)
  find_package(SFML COMPONENTS main graphics system REQUIRED)
ELSE()
  find_package(SFML COMPONENTS graphics system REQUIRED)
ENDIF()

find_package(Threads)

set(SOURCE
	transform.cpp
	main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${GLM_INCLUDE_DIR})

IF (WIN32)
  target_link_libraries(${PROJECT_NAME} PUBLIC sfml-main sfml-graphics sfml-system)
ELSE()
  target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads sfml-graphics sfml-system)
ENDIF()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
