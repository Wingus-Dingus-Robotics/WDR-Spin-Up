#!/bin/bash

# Configure to cross-compile with VEXcode tools
# TOOL_FOLDER="~/Library/Application Support/Code/User/globalStorage/vexrobotics.vexcode/tools/cpp/toolchain_osxarm64"
TOOL_FOLDER="./vex/tools/cpp/toolchain_osxarm64"

CLANG_PATH=$TOOL_FOLDER/clang/bin
GCC_PATH=$TOOL_FOLDER/gcc/bin
MAKE_PATH=$TOOL_FOLDER/tools/bin
export PATH=$CLANG_PATH:$GCC_PATH:$MAKE_PATH:$PATH

# make!
# VEX_SDK_PATH="~/Library/Application Support/Code/User/globalStorage/vexrobotics.vexcode/sdk/cpp/V5/V5_20220726_10_00_00"
VEX_SDK_PATH="./vex/sdk/cpp/V5/V5_20220726_10_00_00"
if [ $1 == "build" ] ; then
  make T=$VEX_SDK_PATH
elif [ $1 == "rebuild" ] ; then
  make clean
  make T=$VEX_SDK_PATH
elif [ $1 == "clean" ] ; then
  make clean
elif [ $1 == "bear" ] ; then
# Generate compile-commands.json using bear
# (assumes bear is already installed and accessible in shell environment)
  make clean
  bear -- make T=$VEX_SDK_PATH
else
  echo "No command given. Try build."
fi
