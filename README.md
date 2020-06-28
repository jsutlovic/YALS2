# YALS2
A pure C implementation of Conway's Game of Life with OpenGL 3 for
visuals

### Features
- Compact data storage format
- Threadsafe data structure
- Half-step visuals

### Requirements
- CMake 3.13+
- SDL2, SDL2_ttf
- OpenGL (3.3+ core profile), GLEW

### Setup
```bash
./build.sh Release
cmake --build build --config Release
../bin/YALS2
```

### Usage

#### Commandline Params
```
-p
    Profile mode: Runs a 200x200 world for <i>*1000 iterations. Default
    is 1000 iterations.

-i <iterations>
    Iterations in profile mode. Multiplied by 1000 for total iterations.
    Default is 1 (1000 iterations).

-t
    Text mode. Don't start graphical version of world output, only
    textual. Prints 5 iterations of the world to stdout. Used primarily
    for debugging purposes.

-n <fill_type>
    Fill type number. Avaliable options are:
    0:  Empty
    1:  Single cell (top left)
    2:  Even in row (vertical lines)
    3:  Checkerboard 1 (even)
    4:  Diagonal lines
    5:  Odd in row (vertical lines)
    6:  Checkerboard 2 (odd)
    7:  Horizontal lines (even)
    8:  Horizontal lines (odd)
    9:  Full fill
    10: Random fill

    This option is useful for initial setup without a pre-existing
    world. This option is ignored if a file is specified and exists.
    This option applies to profile mode as well as non-profile mode.

-w, -x <width>
    Width (horizontal size; x limit) of the world. This option is
    ignored if a file is specified and exists.

-h, -y <height>
    Height (vertical size; y limit) of the world. This option is ignored
    if a file is specified and exists.

-f <filename>
    Filename to read world from, and save world to. If reading the file
    fails, a default world is created. The world will be saved with this
    filename even if initial reading failed.
```

#### Mouse bindings
In graphical mode, these mouse actions are available:
- Left click on cell: Invert cell value
- Mouse scroll: Zoom camera in/out

#### Keyboard keys
In graphical mode, these keys are available:
- **Q, Esc:** Quits the game, saving the world if filename was provided.
- **0-9:** Fill with fill type. See CLI options for reference.
- **R:** Random fill.
- **Tab:** Show the overlay (basic information).
- **V:** Toggle vsync.
- **N:** Iterates the world while pressed.
- **M:** Iterate world by a single half-step.
- **Space:** Toggles iterating the world.
- **H:** Toggles half/full step mode. Half step shows the intermediate step
  between each full world iteration.
- **C:** Rotate through available color schemes.
- **Shift+C:** Reverse rotate through color schemes.
- **Ctrl+C:** Copy world to clipboard (base64-encoded).
- **Ctrl+V:** Paste world from clipboard (base64-encoded).
- **O:** Toggle orthographic vs perspective camera.
- **U:** Reset camera.
- **P:** Toggle cell padding (default on).
- **X:** Save world to filename provided by the `-f` parameter.
- **W,A,S,D,arrow keys:** Move camera position relative to world field.

#### Notes
Currently graphical mode is limited to a 1280x720 pixel window.
