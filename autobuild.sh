#!/bin/bash

set -x
rm -rf $(pwd)/build/*
cd $(pwd)/build &&
	cmake ..
	make


# #!/bin/bash

# # 显示执行的命令
# set -x

# # 清理构建目录
# rm -rf $(pwd)/build/*

# # 进入构建目录
# cd $(pwd)/build || { echo "Failed to enter build directory"; exit 1; }

# # 使用 Debug 模式运行 CMake
# # CMAKE_BUILD_TYPE=Debug 启用调试信息
# # CMAKE_CXX_FLAGS_DEBUG 添加额外的调试标志
# cmake -DCMAKE_BUILD_TYPE=Debug \
#       -DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -Wall -fno-inline -fno-omit-frame-pointer" \
#       ..

# # 编译项目
# # -j$(nproc) 使用所有可用的处理器核心加速编译
# make -j$(nproc)

# # 检查编译是否成功
# if [ $? -eq 0 ]; then
#     echo -e "\n\033[32mBuild completed successfully with debug information.\033[0m"
#     echo -e "\033[33mYou can now use GDB for debugging.\033[0m"
#     echo -e "Example: gdb ./bin/ChatServer\n"
# else
#     echo -e "\n\033[31mBuild failed.\033[0m"
#     exit 1
# fi