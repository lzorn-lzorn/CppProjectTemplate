# CMakeTemplate
这是一个Cpp项目的基础模板, 用于编译个人的小型Cpp项目. 版本至少是 C++11
构建系统支持 CMake, XMake 
# Build System
extern/ 文件夹是对外部第三方库
intern/ 文件夹是内部实现的内部库
include/ 是头文件目录
source/  是源码文件库
doc/ 是内部库的实现文档, 使用 md 格式进行实现
## CMake
- extern 和 intern 内部都是模块化的, 每个库应该有一个内部的 CMakeLists.txt 外部直接使用 subdir 命令引入
- include 和 source 内部不需要任何 CMakeLists.txt 文件, 因为会在最外面的文件中"一把梭"

### CMake Option
你可以在CMakeLists中通过开启和关闭一些选项从而获得一些额外的支持

## XMake

# Config Support
## VS code
安装 CMake/XMake 插件
- CMake/XMake
- CMake Language Support (optioanl)
- CMake Highlight (optioanl)
- CMakeTools
安装 LLVM系列的 C++插件: 
- clangd
- LLDB DAP
- CodeLLDB (optioanl)

## NeoVim

# Native Tools Support
该项目内部自带了一些内部的工具, 对应模块参考对应的 Doc 下的同名 md 文件
