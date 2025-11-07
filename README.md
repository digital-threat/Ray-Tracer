# Vulkan Ray Tracer
Real-time Whitted-style ray tracer built with Vulkan ray tracing extensions. This repository started out as a general purpose game engine, which I was writing as part of the specializion course at Futuregames. I ripped out the forward renderer, as well as some other bits, and continued working on the project pivoting towards building a ray tracing engine.
## Build
Requiers Vulkan SDK.
Build with CMake.
⚠️ This is an educational project never tested on a different machine. Having said that, the project should build correctly with MinGW + GCC toolchain. It will also compile with MSVC or Clang without any errors but the application will crash on initialization. You tell me what's wrong, because I have no clue! Best guess is it has something to do with memory alignment.⚠️
