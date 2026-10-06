# OpenGlt Project

A C++ OpenGL project using GLFW, GLM, and Assimp for 3D graphics rendering.

## Prerequisites

- CMake 3.16 or higher
- Visual Studio 2022 (for Windows)
- OpenGL compatible graphics card

## Building the Project

### Option 1: Using the build script (Recommended)

#### Windows (PowerShell)
```powershell
.\build.ps1
```

#### Windows (Command Prompt)
```cmd
build.bat
```

### Option 2: Manual CMake build

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Configure the project:
```bash
cmake .. -G "Visual Studio 17 2022" -A x64
```

3. Build the project:
```bash
cmake --build . --config Debug
```

## Project Structure

```
OpenGlt/
├── CMakeLists.txt          # Main CMake configuration
├── build.bat              # Windows batch build script
├── build.ps1              # PowerShell build script
├── README.md              # This file
├── OpenGlt/               # Source code
│   ├── src/               # Source files
│   │   ├── graphics/      # Graphics-related code
│   │   ├── io/           # Input/Output code
│   │   └── Main.cpp      # Main entry point
│   ├── assets/           # Shaders, textures, models
│   └── glfw3.dll         # GLFW runtime library
├── Linking/               # External libraries
│   ├── include/          # Header files
│   └── lib/              # Library files
└── x64/                  # Visual Studio build output (legacy)
```

## Dependencies

The project uses the following external libraries (included in the `Linking` directory):

- **GLFW**: Window management and OpenGL context creation
- **GLM**: Mathematics library for OpenGL
- **Assimp**: 3D model loading library
- **GLAD**: OpenGL loading library
- **STB**: Image loading library

## Running the Application

After building, the executable will be located at:
```
build/bin/Debug/OpenGlt.exe
```

## Troubleshooting

### Common Issues

1. **CMake not found**: Make sure CMake is installed and added to your PATH
2. **Visual Studio not found**: Install Visual Studio 2022 or modify the generator in the build scripts
3. **Missing DLLs**: The build process automatically copies required DLLs to the output directory
4. **OpenGL errors**: Ensure your graphics card supports OpenGL 3.3 or higher

### Building for Different Configurations

To build in Release mode, modify the build command:
```bash
cmake --build . --config Release
```

## Features

- **Dear ImGui Editor Interface**: Integrated native editor panels for scene management, object inspection, and asset browsing.
- **Drag & Drop 3D Model Loading**: Drag and drop `.gltf`, `.glb`, `.obj`, `.fbx`, `.stl` files directly into the window. The model loads automatically and the camera immediately focuses to show it.
- **File System Browser & Native File Dialog**: Browse directory trees or open Windows native file picker to import any model.
- **Scene Hierarchy & Inspector**: Full tree hierarchy with object selection, transform editing (Position, Rotation, Scale), and object deletion.
- **Duplication (Shift + D)**: Instantly clone any selected 3D object in the GUI or via hotkey.
- **Object Picking & Movement**: Left-click directly on objects in the 3D viewport to select them, and move them with mouse grab (`G`) or transform sliders.
- **Blender/Unity Style Camera Navigation**: Cursor remains free for GUI interaction; hold Right Mouse Button or press `TAB` to enter fly-look camera mode (WASD).

## Controls & Hotkeys

| Action | Control / Hotkey |
| --- | --- |
| **Select Object** | Left-Click on object in viewport or hierarchy |
| **Duplicate Object** | `Shift + D` (or GUI button) |
| **Delete Object** | `Delete` / `Backspace` (or GUI button) |
| **Grab / Move with Mouse** | `G` key (Click/Enter to confirm, Esc to cancel) |
| **Focus Camera on Object** | `F` key (or GUI button) |
| **Fly Camera Look** | Hold Right Mouse Button (or press `TAB` to toggle) |
| **Camera Movement** | `W`, `A`, `S`, `D`, `Space` (Up), `Left Shift` (Down) |
| **Camera Zoom** | Mouse Scroll Wheel |
| **Toggle Spotlight** | `T` |
| **Launch Physics Sphere** | `H` |
| **Drag & Drop File** | Drag any 3D file (.gltf, .obj, etc.) into the window |
