# A utility to draw spheres, vectors and triangles

A program I needed to debug some graphics. I copy and paste groups of coordinates from stdout of the debugged program into the text input in this program.

![A screenshot of this program](https://github.com/sebsjames/tripoints/blob/main/tripoints.png?raw=true)

## Dependencies

If you are using Debian or Ubuntu, the following `apt` command should
install the dependencies.

```bash
sudo apt install build-essential cmake git wget  \
                 nlohmann-java3-dev librapidxml-dev freeglut3-dev libxmu-dev \
                 libxi-dev libglu1-mesa-dev libglfw3-dev libfreetype-dev
```

## Building

To build and run the program make sure to **clone with recurse-submodules**:

```bash
# Clone with recursion
sgit clone git@github.com:sebsjames/tripoints --recurse-submodules

# If you forgot --recurse-submodules, you can now:
# git submodule update --init --recursive
# To get the submodules imgui and mathplot (and nested sebsjames/maths)

# Build it in a 'build' directory
mkdir build
cd build
cmake ..
make
./tripoints # You should see a window containing an ImGui.
```

Try entering these strings into the text field:

```
0,0,0
(0.1,0.1,0)
(0,0,0),(0,0,1)
(0,0,0),(0.5,0,0),(0,0.5,0)
```

## Copy and paste

There's a small possible gotcha. 
If Ctrl-C to copy or Ctrl-V to paste doesn't seem to work in tripoints, check to see if you have Gnome's "Locate Pointer" accessibility feature turned on. 
If so, the Left-Ctrl key press event can fail to make its way into Dear ImGui. 
Workarounds are to use Right-Ctrl or turn off "Locate Pointer".
See the [ImGui issue](https://github.com/ocornut/imgui/issues/9224) and [GLFW issue](https://github.com/glfw/glfw/issues/2826) for more details.
