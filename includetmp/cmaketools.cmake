# usage: 
# add_executable(${PROJECT_NAME} ${sources})
# check_and_target_link_libraries(ENABLE_BENCHMARK ${PROJECT_NAME} benchmark::benchmark)

function(check_and_target_link_libraries option_name target_name lib_name)
    # 获取变量当前值
    if(${option_name})
        target_link_libraries(${target_name} PRIVATE ${lib_name})
    endif()
endfunction()