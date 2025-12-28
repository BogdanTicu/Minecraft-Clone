# Minecraft-style Voxel World (Tema 2)

A C++ OpenGL-based voxel terrain renderer inspired by Minecraft, featuring chunk-based terrain generation, infinite world loading, and block manipulation.

## Features

- **Chunk-based Terrain Generation**: Dynamic world generation using a chunk system (16x16 blocks per chunk)
- **Infinite World**: Chunks are loaded/unloaded dynamically based on player position
- **Procedural Terrain**: Height map generation using noise-based algorithms
- **Block Interaction**:
  - Break blocks with raycasting
  - Place blocks in the world
- **Instanced Rendering**: Efficient rendering of thousands of blocks using OpenGL instancing
- **First-Person Camera**: Free-roaming camera system

## Technical Details

### Technologies Used
- **Language**: C++20
- **Graphics API**: OpenGL
- **Math Library**: GLM (OpenGL Mathematics)

### Project Structure
```
Tema2/
??? Camera.cpp/h      # First-person camera implementation
??? Cube.cpp/h        # Cube mesh and instanced rendering
??? World.cpp/h       # Chunk management and terrain generation
??? main.cpp          # Application entry point and game loop
??? simple.vert/frag  # GLSL shaders
??? include/          # External libraries headers
??? lib/              # External libraries binaries
??? src/              # Additional source files
```

### Key Components

#### World System
- **Chunk Size**: 16x16 blocks
- **Render Distance**: Configurable chunk loading radius
- **Block Types**: Grass (green) and stone (gray) blocks
- **Noise Generation**: Custom procedural height map using trigonometric functions

#### Rendering
- Uses instanced rendering to draw multiple blocks in a single draw call
- Separate rendering passes for grass and stone blocks
- Simple color-based block differentiation

#### Player Interaction
- **Break Block**: Raycasting from camera position to detect and remove blocks
- **Place Block**: Places a block in front of the first detected block
- Maximum reach: 5 units

## Building the Project

### Requirements
- Visual Studio 2019 or later
- C++20 compatible compiler
- OpenGL 3.3+ capable graphics card

### Build Instructions
1. Open `Tema2.sln` in Visual Studio
2. Ensure all dependencies are properly linked (GLM, GLEW, GLFW, etc.)
3. Build the solution (Ctrl+Shift+B)
4. Run the executable

## Controls
*(Add your specific controls here, for example:)*
- **W/A/S/D**: Move camera
- **Mouse**: Look around
- **Left Click**: Break block
- **Right Click**: Place block
- **ESC**: Exit application

## Future Improvements
- Multiple block types and textures
- Improved noise generation (Perlin/Simplex noise)
- Lighting system
- Water and transparent blocks
- Chunk serialization (save/load world)
- Multithreaded chunk generation

## License
This project was created as part of a computer graphics course (Anul 3, FMI).

## Author
Created for Grafica Tema 2
