#!/bin/bash

cd "$(dirname "$0")"

if ! command -v brew &> /dev/null; then
    echo "Homebrew not found. Please install Homebrew first."
    exit 1
fi

brew install cmake
brew install sdl2
brew install doxygen

if [ ! -d "external/imgui" ]; then
    git clone https://github.com/ocornut/imgui.git external/imgui
else
    echo "ImGui repository already exists."
fi

cd external/imgui
git fetch --all
git checkout docking || git checkout -b docking origin/docking
git pull origin docking

cd ../../

echo "Dependencies setup completed."
