# A utility to draw spheres, vectors and triangles

A program I needed to debug some graphics. I copy and paste groups of coordinates from stdout of the debugged program into the text input in this program.

![A screenshot of this program](https://github.com/sebsjames/tripoints/blob/main/tripoints.png?raw=true)

## Dependencies

If you are using Debian or Ubuntu, the following `apt` command should
install the mathplot dependencies. Note that `libarmadillo-dev`
and `libhdf5-dev` are optional. They're not used by `prog1.cpp` but they
do allow all the mathplot headers to be used in this template.

```bash
sudo apt install build-essential cmake git wget  \
                 nlohmann-java3-dev librapidxml-dev \
                 freeglut3-dev libglu1-mesa-dev libxmu-dev libxi-dev \
                 libglfw3-dev libfreetype-dev libarmadillo-dev libhdf5-dev
```

## Building

To build and run the example:

```bash
# Clone this example
git clone git@github.com:sebsjames/tripoints

# Clone, copy or symlink mathplot INSIDE your example:
cd tripoints # or whatever you named your fork/copy
git clone --recurse-submodules git@github.com:sebsjames/mathplot

# Clone, copy or symlink Dear ImGui INSIDE your example:
git clone git@github.com:ocornut/imgui

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
(0,0,0),(0.5,0,0),(0,0,5,0)
```
