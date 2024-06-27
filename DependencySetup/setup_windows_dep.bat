@echo off

cd "%~dp0"

where choco >nul 2>nul
IF %ERRORLEVEL% NEQ 0
(
    echo Chocolatey not found. Please install Chocolatey first.
    exit /b 1
)

choco install cmake -y
choco install sdl2 -y
choco install doxygen.install -y

if not exist "external\imgui"
(
    git clone https://github.com/ocornut/imgui.git external\imgui
)
else
(
    echo ImGui repository already exists.
)

cd external\imgui
git fetch --all
git checkout docking || git checkout -b docking origin/docking
git pull origin docking

cd ..\..\

echo Dependencies setup completed.
pause
