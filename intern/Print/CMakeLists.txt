

set(TARGET_NAME Print)

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
target_include_directories(${TARGET_NAME} INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Base
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Include
)

