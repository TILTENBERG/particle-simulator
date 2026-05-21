# 🌌 Cyberpunk Collision Simulator

A high-performance, multithreaded 2D particle physics engine and interactive collision simulator built with **C++** and **SFML 3.0.2**. Features a premium cyberpunk setup dashboard, zero-stall rendering, inertial window coupling, and a dynamic dual slider-keyboard input control system.

---

## ⚡ Core Features

*   **$O(N)$ Spatial Grid Partitioning:** Divides the 800x800 arena into uniform $40\text{px} \times 40\text{px}$ cells. Particles only test collisions against their own cell and 8 adjacent neighbors, dropping the complexity from $O(N^2)$ to $O(N)$ for high particle counts.
*   **Dual-Input Premium Sliders:** Offers horizontal drag-sliding with fluid micro-animations (handle scaling, progress filling, color shifts) alongside **direct keyboard numeric entry** when clicking on a parameter's right-aligned numerical value box.
*   **Dynamic Parameter Randomization:** Toggle checkboxes to switch between completely randomized particle radius/mass distributions or uniform constant properties. The sliders' titles adjust dynamically in real time (e.g., *Max Radius* $\leftrightarrow$ *Constant Radius*).
*   **Zero-Stall Multithreading:** Physics updates and SFML rendering run on an independent background thread. This keeps the simulation running at uncapped frame rates and prevents particle freezes or window-dragging stalls on Windows OS.
*   **Inertial Frame Forces:** Moving or shaking the desktop simulation window injects physical kinetic energy and opposite-direction impulse force vectors directly to all particles inside.
*   **Realistic Physics Integration:** Downward constant gravity, inelastic particle-to-particle momentum transfers using restitution coefficients, and sliding friction forces ($0.98$ friction) causing resting piles at the floor.

---

## 🎮 Interface & Interactive Controls

### 🔧 Pre-Simulation Setup Menu
*   **Particle Count:** Adjusts total spheres simulated (from $10$ to $1000$).
*   **Radius (px):** Sets the radius slider range (from $5\text{px}$ to $45\text{px}$). Toggle the *Randomize Radius* checkbox to switch between a random size distribution or uniform constant sizes.
*   **Mass (kg):** Sets the mass slider range (from $1\text{kg}$ to $100\text{kg}$). Toggle *Randomize Mass* to choose between random mass allocations or uniform constant masses.
*   **Restitution (Bounciness):** Configures physical bounciness (from $0.00$ for totally inelastic collisions to $1.00$ for fully elastic bounces).

### ⌨️ Input Modes
*   **Mouse Interaction:** Click and drag slider handles, toggle checkboxes, or hover over the neon **Start Simulation** button.
*   **Keyboard Numeric Input:**
    1.  **Click** on the right-aligned cyan numeric value box of any slider.
    2.  The text highlights to **amber yellow** with an active cursor (`_`) and a neon underline.
    3.  Type numbers directly (only digits and a single decimal point are validated; letters are blocked).
    4.  Press **Enter** (or click outside) to commit and validate the changes safely within boundaries.
    5.  Press **Escape** to cancel editing and revert to the previous value.
*   **Simulation Controls:** Press **ESC** during the active simulation to cleanly terminate the background thread and return back to the parameters menu.

---

## 🛠️ Build & Compilation

The project statically links against **SFML 3.0.2** and is compiled natively on Windows using MinGW64 `g++`.

### Compilation Command
Run the following command from your terminal inside the project directory:

```powershell
C:\mingw64\bin\g++.exe -fdiagnostics-color=always -g -DSFML_STATIC -IC:\SFML-3.0.2\include main.cpp -LC:\SFML-3.0.2\lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32 -lws2_32 -mwindows -o main.exe
```

### Compiler Flag Explanations:
*   `-DSFML_STATIC`: Declares that SFML libraries are linked statically to produce a single standalone executable.
*   `-IC:\SFML-3.0.2\include`: Specifies the path to SFML header files.
*   `-LC:\SFML-3.0.2\lib`: Specifies the path to static `.a` library binaries.
*   `-lsfml-graphics-s -lsfml-window-s -lsfml-system-s`: Links the core static SFML modules.
*   `-lopengl32 -lfreetype -lwinmm -lgdi32 -lws2_32`: Links standard Windows system dependencies required by static SFML.
*   `-mwindows`: Compiles a clean release binary that launches the GUI directly without spawning a background cmd console.

---

## 📂 File Architecture

*   📄 **[main.cpp](file:///c:/Users/bilgu/OneDrive/Documents/Projects/Game_Project/main.cpp):** Manages the core setup menu state-machine, keyboard-focused sliders, interactive button rendering, and the multi-threaded physics thread initialization.
*   📄 **[Particle.hpp](file:///c:/Users/bilgu/OneDrive/Documents/Projects/Game_Project/Particle.hpp):** Defines particle parameters, constant gravity vector, inelastic momentum solving formulas, sliding wall friction multipliers, and rendering shapes.
*   📄 **[Partition.hpp](file:///c:/Users/bilgu/OneDrive/Documents/Projects/Game_Project/Partition.hpp):** Implements the $O(N)$ broad-phase uniform spatial partitioning grid.
*   📄 **[Vector2D.hpp](file:///c:/Users/bilgu/OneDrive/Documents/Projects/Game_Project/Vector2D.hpp):** Handles two-dimensional physics math, dot products, vector lengths, normalize routines, and operations.
