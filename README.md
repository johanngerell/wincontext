# wincontext

Benchmarking different methods to store Win32 per-window context

## Building with CMake

### Using the Ninja generator

The **Ninja** CMake generator is a single-configuration generator. This means that separate CMake configurations are used for each of _Debug_, _Release_, and _RelWithDebInfo_ builds.

Create a Debug configuration:

    > mkdir build\debug
    > cd build\debug
    > cmake ..\.. -DCMAKE_BUILD_TYPE=Debug -GNinja

Build all targets in Debug:

    > cd build\debug
    > cmake --build .

Build one target in Debug:

    > cd build\debug
    > cmake --build . --target tests

## Submodules

Update all submodules in `submodules/` with

    > git submodule update --recursive --remote

Not sure what `--merge` does here, but it might be needed.

    > git submodule update --recursive --remote --merge
