# BABEL

OpenGL library with infinite rooms. Walk through doorways to loop back to the same space.

Based on Borges' Library of Babel concept.

## Build Instructions

Windows only. Requires Visual Studio.

1. Open `BABEL.sln`
2. Set platform to x64
3. Build and run

## Controls

- WASD + mouse: move
- Space/Ctrl: up/down
- P: toggle portals
- M: drama lighting
- H: help

## Features

- Portal rendering for infinite space illusion
- 3D models with PBR textures
- Light sources with realistic attenuation
- Animated floating books and orbiting torches
- Debug system (F1-F5, F10)

## How it works

Portal system renders the scene from multiple camera angles into textures, then maps those textures onto doorway surfaces. Walking through teleports you back to create seamless infinite rooms.
