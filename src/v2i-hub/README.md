# Introduction

The **v2i-hub** directory contains C++ Plugins written for **V2X Hub**. Included here is a **Plugin Programming Guide** which includes and [sample plugin](../../examples/tmx-exampleapps/SampleBSMPlugin/) that can be used as a reference  for developing new V2X-Hub Plugins

**[Programming Guide](./docs/Programming_Guide.md)**

### Custom Plugin Selection (Manual Local Build)

The script defaults to pulling from Docker Hub (all plugins) if no local image exists. To create a custom image with a certian set of plugins:

1. From the repo root:  
   ```
   docker build --build-arg SKIP_PLUGINS="Plugin1 Plugin2" -t usdotfhwaops/v2xhub:develop .
   ```
   - SKIP_PLUGINS is space-separated plugin names to skip over (case-sensitive, matching directory names in src/v2i-hub).
   - Empty or omitted = build all.

2. Run `./initialization.sh` — uses your local image if the tag matches.