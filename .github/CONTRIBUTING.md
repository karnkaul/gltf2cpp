# Contributing to facade

:+1::tada: First off, thanks for taking the time to contribute! :tada::+1:

The following is a set of guidelines for contributing to repositories hosted in the [cpp-gamedev Organization](https://github.com/cpp-gamedev) on GitHub. These are mostly guidelines, not rules. Use your best judgment, and feel free to propose changes to this document in a pull request.

#### Table Of Contents

[Code of Conduct](#code-of-conduct)

[What should I know before I get started?](#what-should-i-know-before-i-get-started)
  * [Philosophy](#philosophy)
  * [Development](#development)

[Project Layout](#project-layout)
  * [Targets](#targets)
  * [Library vs Application](#library-vs-application)

[Styleguides](#styleguides)
  * [Git Commit Messages](#git-commit-messages)
  * [C++ Styleguide](#c-styleguide)
  * [Documentation Styleguide](#documentation-styleguide)

## Code of Conduct

This project and everyone participating in it is governed by the [facade Code of Conduct](../CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

* [Github Discussions](https://github.com/cpp-gamedev/facade/discussions)

## What should I know before I get started?

### Philosophy

`facade` aims to be lightweight, simple, performant, and follow modern best practices. It builds everything from source to have maximum target platform coverage without having to ship/require third-party binaries. It uses the subset of C++20 currently supported by CMake and the three major compilers (GCC, Clang, MSVC): no coroutines or modules, and only limited use of ranges. It packs external dependencies into an archive to circumvent git submodules / CMake FetchContent calls, and for the CMake configure step and CI builds to be fast. It provides a number of useful default presets in `CMakePresets.json`. Contributors are urged to align with these ideas / examples.

### Development

The following guidelines are strongly recommended during development:

1. Turn on Vulkan validation layers: Use Vulkan Configurator and enable all validation types (except Best Practices). Keep it running while developing / debugging `facade`. You will need to have Vulkan SDK in `PATH` (or export `VULKAN_SDK` etc).
1. UBSan: ideally, work with UB Sanitizer enabled to catch inadvertent bugs. ASan will unfortunately trigger lots of false positives from the video driver, windowing system, etc. The CMake preset `ninja-ubsan` offers this out of the box.

## Project Layout

```
cmake/              CMake interface targets, utility scripts, etc
ext/                root for external dependencies
facade-lib/         root for core library code
  include/facade    interface for core library
  src               core library sources (and internal headers)
misc/               miscellaneous (not required for the project)
shaders/            root for embedded shaders and API for them
src/                root for application code
tools/              root for facade tools
  embed_shader      embed-shader tool source
```

### Targets

`facade` is split into the core library (`facade-lib/`), and the application (`src`). Most of the code resides in the core library as API types, with them being used in a render loop in the application code. The app code also embeds the shaders it uses - the library expects compiled SPIR-V. This is done via a custom tool - `embed-shader` (`tools/`), which invokes `glslc` (required in `PATH`) to compile GLSL to SPIR-V, and then "burns" that into generated C++ headers which the app then includes. The dependencies are set up in CMake so any changes to GLSL sources trigger re-embedding them.

### Library vs Application

Most prototype code will be tested on the app side, perhaps with throwaway code, before being "upgraded" to the core library, unless it belongs on the app side to begin with. Where to draw that line isn't a very sharp line, but the general idea is to keep the core library fully featured yet generic enough to be pulled out into another project / app while giving that app as much leeway as possible to deviate in its targets from this one. As is evident, it is mainly a balancing act both of those ideals, along with convenience, simplicity, and time required to get something done.

## Styleguides

### Git Commit Messages

* Use the present tense ("Add feature" not "Added feature")
* Use the imperative mood ("Move cursor to..." not "Moves cursor to...")
* Limit the first line to 72 characters or less
* Reference issues and pull requests liberally after the first line
* When only changing documentation, include `[ci skip]` in the commit title
* Consider starting the commit message with an applicable emoji:
    * :art: `:art:` when improving the format/structure of the code
    * :racehorse: `:racehorse:` when improving performance
    * :memo: `:memo:` when writing docs
    * :penguin: `:penguin:` when fixing something on Linux
    * :apple: `:apple:` when fixing something on macOS
    * :checkered_flag: `:checkered_flag:` when fixing something on Windows
    * :bug: `:bug:` when fixing a bug
    * :fire: `:fire:` when removing code or files
    * :green_heart: `:green_heart:` when fixing the CI build
    * :white_check_mark: `:white_check_mark:` when adding tests

### C++ Styleguide

All C++ code is formatted with [clang-format](https://clang.llvm.org/docs/ClangFormat.html).

* Use `snake_case` for variables, functions, etc, and `PascalCase` for types, concepts, etc.
* Use `eFoo` for enum values to sidestep any possible keyword collisions
* Use west const (`int const` vs `const int`)
* Minimize includes in headers
  * Use forward declarations where possible except at boundary code (user should not have to include another header to use an API)
* Avoid platform-dependent code

### Documentation Styleguide

* Follow existing Doxygen style doc comments
* Boundary code should have doc-comments unless code is self-documenting
* Doxygen integration and docs generation pending
