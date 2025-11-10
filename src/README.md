# V2X Hub Build

This directory contains the build scripts as well as the source code required for building V2X Hub.


## Build

The `build.sh` script here is used to build V2X Hub. The documentation is provided below or can be accessed using the `-h` parameter.

```bash
Usage: ./build.sh [BUILD_TYPE] [--j2735-version <version>] [--skip-plugins 'list']

Required positional arguments:
  BUILD_TYPE            The build type (e.g., debug, release, coverage)

Required options:
  --j2735-version INT   Specify the J2735 version as an integer (e.g., 2016, 2020, 2024)

Optional options:
  --skip-plugins STRING Space-separated list of plugin directory names to skip (case-sensitive, default empty = build all)
  -h, --help            Show this help message and exit

If arguments are not provided, the script will prompt interactively.
```
DockerHub hosted V2X Hub images include all plugins by default. Instructions below can be used to build lightweight images for a subset of plugins.

1. From the repo root:  
   ```
   docker build --build-arg SKIP_PLUGINS="Plugin1 Plugin2" -t usdotfhwaops/v2xhub:<custom-tag> .
   ```
   - SKIP_PLUGINS is space-separated plugin names to skip over (case-sensitive, matching directory names in src/v2i-hub).
   - Empty or omitted = build all.
2. Update `configuration/docker-compose.yml` to point to built image `usdotfhwaops/v2xhub:<custom-tag>` for v2xhub service.

3. Run `docker compose up -d`

## Testing and Coverage

As part of our CI/CD QA we use code coverage tools to track the source code coverage of our unit test. The `install_converage_depedencies.sh` installs dependencies for this process. 

The `./test.sh` script will run all the unit tests and if passed the coverage parameter (e.g `./test.sh coverage`) it will evaluate source code coverage and produce relevant report. 