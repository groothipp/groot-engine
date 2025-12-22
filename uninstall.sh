#!/bin/bash

# Groot Engine Uninstall Script

echo "Uninstalling Groot Engine..."

# Remove header files
if [ -d "/usr/local/include/groot" ]; then
  echo "Removing /usr/local/include/groot..."
  sudo rm -rf /usr/local/include/groot
else
  echo "Headers not found in /usr/local/include"
fi

# Remove library files
if [ -f "/usr/local/lib/libgroot.a" ]; then
  echo "Removing /usr/local/lib/libgroot.a..."
  sudo rm -f /usr/local/lib/libgroot.a
elif [ -f "/usr/local/lib/libgroot.so" ]; then
  echo "Removing /usr/local/lib/libgroot.so"
  sudo rm -f /usr/local/lib/libgroot.so
elif [ -f "/usr/local/lib/libgroot.dylib" ]; then
  echo "Removing /usr/local/lib/libgroot.dylib"
  sudo rm -f /usr/local/lib/libgroot.dylib
else
  echo "Library not found in /usr/local/lib"
fi

# Remove CMake package files
if [ -d "/usr/local/lib/cmake/GrootEngine" ]; then
  echo "Removing /usr/local/lib/cmake/GrootEngine..."
  sudo rm -rf /usr/local/lib/cmake/GrootEngine
else
  echo "CMake package files not found"
fi
