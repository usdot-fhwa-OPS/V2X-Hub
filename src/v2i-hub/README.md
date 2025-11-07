# Introduction

The **v2i-hub** directory contains C++ Plugins written for **V2X Hub**. Included here is a **Plugin Programming Guide** which includes and [sample plugin](../../examples/tmx-exampleapps/SampleBSMPlugin/) that can be used as a reference  for developing new V2X-Hub Plugins

**[Programming Guide](./docs/Programming_Guide.md)**

## Plugin Selection

V2X-Hub supports selective building of plugins based on the user's need. This is controlled at the CMake level via options in [`CMakeLists.txt`](CMakeLists.txt) and passed through the build script and Dockerfile.

### Usage
#### Via [`../build.sh`](../build.sh) (Local Build)
Inside the build script, there is a new `--plugins` flag:

- Build all: `./build.sh release --j2735-version 2024 --plugins "All"`
- Build specific: `./build.sh release --j2735-version 2024 --plugins "PluginsHere"`

This generates CMake flags like `-DBUILD_PLUGIN_PluginsHere=ON` (and sets others to OFF), which is then passed to CMake to build only the selected plugins. 

#### Via Initialization Script
When running `../../configuration/initialization.sh`:
- It prompts for plugins (space-separated or "All"). The initialization script defaults to building all of the plugins if nothing is inputted in the prompt.
- Writes to .env as PLUGIN_INPUT.
- This will build a new image of V2X Hub with only the selected plugins 