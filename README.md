# 🖥️ OSAS: Operating System As a Software

> An experimental, bare-metal 16-bit Operating System designed from scratch to test a hardcore engineering hypothesis: **Can we achieve maximum computational efficiency by building a complex mathematical application as its own independent, low-level OS?**

## 📖 Overview & Hypothesis
Traditionally, resource-intensive applications run on top of thick abstraction layers (High-level language → Runtime/GC → OS Kernel → Hardware). **OSAS** eliminates the middleman. By stripping away modern abstractions, standard libraries, and background OS processes, this project explores the performance and architectural behavior of software running directly on the hardware. 

## ⚙️ Bare-Metal Architecture
Everything in OSAS runs in **16-bit Real Mode** by design. 
* **Zero Standard Libraries (`libc`):** The system is compiled completely independent of standard C libraries. Every basic function, from string manipulation to formatted output (`stdio.c`), is written from scratch.
* **Custom Hardware Abstraction:** Low-level CPU interactions and I/O are handled through custom Assembly routines (`x86.asm`).
* **File System:** Implements a fully functional **FAT16** driver. FAT16 was chosen for its optimal balance of architectural simplicity and reliable developmental flexibility, allowing the OS to interact with real disk structures.

## 🧮 The Crown Jewel: FEM Solver (`calc`)
The ultimate test of OSAS is its ability to solve a 1D mechanical task (elastic deformation of a rod) using the **Finite Element Method (FEM)**. Doing this in 16-bit real mode presented massive challenges:
* **No Math Coprocessor (FPU):** The solver uses purely integer-based fixed-point arithmetic to maintain 3 decimal places of precision without floating-point hardware.
* **Overcoming 16-bit Limitations:** To prevent integer overflow when assembling the global stiffness matrix and solving the SLAE (System of Linear Algebraic Equations), I wrote custom **32-bit Assembly macros** (`__I4M`, `__I4D`) that merge 16-bit registers to perform 32-bit multiplication and division on the fly.
* **Iterative Solver:** Implements the Gauss-Seidel method with mathematical rounding to nearest integers to eliminate "integer truncation drift," successfully converging perfectly purely on bare-metal math.

## 🛠️ Features & CLI Commands
The OS includes a basic shell capable of navigating the FAT16 file system and launching the mathematical engine.

| Command | Description |
| :--- | :--- |
| `help` | Displays a list of all available terminal commands. |
| `clear` | Clears the video memory/screen. |
| `ls` | Lists directories and files in the current FAT16 path. |
| `mkdir <name>` | Creates a new directory on the disk. |
| `cd <path>` | Changes directory. |
| `read <path>` | Reads and displays the contents of a text file. |
| `calc` | **Launches the 1D FEM solver**, calculating node displacements using the Gauss-Seidel method. |
| `poweroff` | Safely halts the CPU and shuts down the system. |

## 🚀 Build & Run

**Requirements:** `make`, `nasm`, `open-watcom` (wcc, wlink), `qemu-system-x86_64`.
```bash
# Clone the repository
git clone https://github.com/R0sale/OS.git
cd OS

# Build the binary image (Bootloader + Kernel)
make 

# Run in terminal
./run.sh
