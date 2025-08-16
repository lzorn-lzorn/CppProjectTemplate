

set(TARGET_NAME extra_functions)

# file(GLOB_RECURSE
# INC
# *.h
# *.hpp
# )
# file(GLOB_RECURSE
# SRC
# *.cpp
# )

# add_library(${TARGET_NAME} ${INC} ${SRC})
add_library(${TARGET_NAME} INTERFACE)
target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
