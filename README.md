# ForgeLibrary

A lightweight, high-performance C utility library providing arena
allocators, thread-safe memory tracking with canary bounds checks, lock 
free/POSIX data structures, file I/O, and job dispatch. Mainly for my 
personal projects.

--- 

## Features
* **Memory Management**: 
  * Arena (`ForgeLinearAllocator`), 
  * Fixed-Size Pool (`ForgeObjectPool`), 
  * Canary Memory Tracker with leak reporting.

* **Data Structures**: 
  * ForgeDynamicArray, 
  * ForgeHashMap (FNV-1a / Open Addressing), 
  * ForgeAVLTree (Ordered Set), 
  * ForgeQueue, 
  * ForgeStack, 
  * ForgeRingBuffer.

* **System Utilities:** 
  * Multi-threaded ForgeThreadPool, cross-platform 
  * POSIX File I/O (ForgeFile), and zero-allocation directory iteration.

* Diagnostics: Configurable assertion macros and level-based logging (FORGE_LOG_*).


--- 

## Quick Start
* **Building with CMake**
```Bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

* **Running Examples**
All module guides and usage examples are built into the examples/ binary folder:

```Bash
# Memory Tracker & Canary Checks
./examples/trackerExample

# Thread Pool Multi-threading
./examples/threadPoolExample

# Linear Allocator Arena
./examples/linearAllocExample
```

--- 

## Documentation & Examples
For full function signatures, data structure APIs, and detailed usage
patterns, see the Documentation Index and examine the code in examples/.
