# Vaslet

Vaslet is a Linux-focused C++ project for a lightweight JSON configuration
center.

## Quick Start

Requirements: Linux, Git, CMake 3.16 or later, a C++17 compiler (GCC or Clang),
and a build tool such as Make or Ninja.

After cloning the repository, initialize the dependency from the project root:

```sh
git submodule update --init thirdparty/rapidjson thirdparty/plog
```

From the project root, initialize the dependency and build the Release target:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target vasletd --parallel
```

The configured executable output path is `build/bin/vasletd`:

```sh
build/bin/vasletd -v
build/bin/vasletd -h
build/bin/vasletd -c config/vasletd_conf.json
```

- `-v` prints the project version.
- `-h` prints help.
- `-c <file>` selects an explicit configuration path. Relative paths use the
  working directory. The last `-c` wins if repeated.
- Without `-c`, the executable loads the project's installed default
  configuration. It does not search the working directory for a configuration.

## Installation

Set the final installation prefix when configuring, then build and install:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build --target vasletd --parallel
sudo cmake --install build
/usr/local/bin/vasletd
```

## Tests

Unit tests live under `tests/units` and use GoogleTest exclusively. When
`BUILD_TESTING=ON`, CMake fetches GoogleTest v1.18.0 at a pinned commit and builds
`vaslet_unit_tests`.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --target vaslet_unit_tests --parallel
ctest --test-dir build --output-on-failure
```

## License

Vaslet is licensed under the [MIT License](LICENSE).
RapidJSON's license and bundled third-party notices are available in
[thirdparty/rapidjson/license.txt](thirdparty/rapidjson/license.txt).
The cmdline library retains its upstream BSD three-clause terms and
copyright notice; see [thirdparty/cmdline/LICENSE](thirdparty/cmdline/LICENSE).
Installation includes `cmdline-LICENSE.txt` in `share/vaslet` (under the configured
install data directory) for binary redistribution. Retain these notices when
packaging the executable separately.
plog is distributed under the MIT License; see
[thirdparty/plog/LICENSE](thirdparty/plog/LICENSE). Installation includes
`plog-LICENSE.txt` alongside the other dependency licenses.

