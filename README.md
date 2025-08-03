# CMakeTemplate
这是一个CMake基础模板, 用于使用CMake编译个人的小型项目

extern/ 文件夹是对外部第三方库
intern/ 文件夹是内部实现的内部库

以上两种库都是模块化的, 每个库应该有一个内部的 CMakeLists.txt 外部直接使用 subdir 命令引入

include/ 是头文件目录
source/  是源码文件库
以上两种内部不需要任何 CMakeLists.txt 文件, 因为会在最外面的文件中"一把梭"

doc/ 是内部库的实现文档, 使用 md 格式进行实现