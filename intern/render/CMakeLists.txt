set(TARGET_NAME render)

file(GLOB_RECURSE
INC
    *.h
    *.hpp
)
file(GLOB_RECURSE
SRC
    *.cpp
)

add_library(${TARGET_NAME} ${INC} ${SRC})
target_include_directories(${TARGET_NAME} PRIVATE ${Vulkan_INCLUDE_DIR})
# add_library(${TARGET_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} PRIVATE render)
target_include_directories(${PROJECT_NAME} PUBLIC ${INC})
