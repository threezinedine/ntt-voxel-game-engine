# meed-voxel-game-engine

## Requirements

- Python 3.11 or higher
- CMake 3.20 or higher
- [Emscripten SDK (for WebAssembly builds)](https://emscripten.org/)

## Installation

### Linux

1.Clang-format:

```bash
sudo apt install clang-format
```

2.Install Vukan validationi layers:

```bash
sudo apt install vulkan-tools
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-noble.list https://packages.lunarg.com/vulkan/lunarg-vulkan-noble.list
sudo apt update
sudo apt install vulkan-sdk
```

Todo:

- [x] Vertex buffer for OpenGL backend
- [x] Transfer vertex buffer Vulkan backend
- [ ] Using multiple viewport for pyqt6 editor
- [ ] Add file watcher to reload assets on change (Python)
- [ ] Implement python tests for the Pyqt editor
