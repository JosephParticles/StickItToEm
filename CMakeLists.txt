﻿cmake_minimum_required (VERSION 3.10)

project (Game VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#Add main source file
list(APPEND ALLSOURCES "src/Main.cpp")

#Add all engine source files
list(APPEND ALLSOURCES "src/Engine/BaseLevel.cpp" "src/Engine/BaseLevel.h")
list(APPEND ALLSOURCES "src/Engine/Program.cpp" "src/Engine/Program.h")
list(APPEND ALLSOURCES "src/Engine/Timer.cpp" "src/Engine/Timer.h")
list(APPEND ALLSOURCES "src/Engine/Input.cpp" "src/Engine/Input.h")
list(APPEND ALLSOURCES "src/Engine/MathsFunctions.cpp" "src/Engine/MathsFunctions.h")
list(APPEND ALLSOURCES "src/Engine/TextureManager.cpp" "src/Engine/TextureManager.h")

#Add all game source files
list(APPEND ALLSOURCES "src/Game/GameLevel.cpp" "src/Game/GameLevel.h")
list(APPEND ALLSOURCES "src/Game/TerrainManager.cpp" "src/Game/TerrainManager.h")

add_executable (Game ${ALLSOURCES})

#Add all the header files needed
target_include_directories(Game PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/bin/includes")	

#Add GLFW library (compiled .lib file)
add_library(GLFW STATIC IMPORTED GLOBAL)
set_property(TARGET GLFW PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/bin/GLFW/lib/glfw3.lib")

#Add GLAD library (uncompiled .c file)
add_library(GLAD "${PROJECT_SOURCE_DIR}/bin/GLAD/src/glad.c")
target_include_directories(GLAD PUBLIC "${PROJECT_SOURCE_DIR}/bin/includes")

#Add GLM files (not compiled as a library cause it's all header files)
target_include_directories(Game PUBLIC "${PROJECT_SOURCE_DIR}/bin/glm")

#Link the libraries
target_link_libraries(Game PUBLIC GLFW)
target_link_libraries(Game PUBLIC GLAD)


#Copy the shaders into the output directory
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/Shaders")

add_custom_target(copyShaders ALL COMMAND ${CMAKE_COMMAND} -E copy_directory 
"${CMAKE_SOURCE_DIR}/src/Shaders" "${CMAKE_BINARY_DIR}/src/Shaders" COMMENT
"Copy shaders to output file" VERBATIM)

add_dependencies(Game copyShaders)



