# GitHub Copilot / AI agent instructions for OpenCPN

Purpose: give an AI coding agent the minimal, actionable context to be productive working in this repository.

- **Big picture**: OpenCPN is a C++17, wxWidgets-based navigation application composed of a core (GUI and platform glue) and a plugin system.
  - Core areas: `gui/` (UI), `model/` (platform, plugin infrastructure, data models), `include/` (public headers), and `plugins/` (third-party plugins).
  - Plugin loading is implemented in `model/src/plugin_loader.cpp` and runtime/search paths are in `model/src/plugin_paths.cpp`.
  - Plugins implement versioned plugin interfaces (examples: `opencpn_plugin_117`) defined by `ocpn_plugin.h`/headers under `include/` and `model/`.

- **Build & test (practical)**:
  - Typical local Linux build:
    ```bash
    mkdir build && cd build
    cmake ..
    make -j$(nproc)
    sudo make install
    ```
  - Enable tests: `cmake -DOCPN_BUILD_TEST=ON ..` then `make -j` and run `ctest -V` or `make test`.
  - Important CMake knobs: `OCPN_USE_BUNDLED_LIBS`, `OCPN_BUILD_TEST`, `OCPN_ENABLE_CLANG_TIDY`, `ENABLE_SANITIZER`, `OCPN_TARGET_TUPLE` (Android cross-build). See top-level `CMakeLists.txt` for defaults and other options.
  - Dependency notes: requires wxWidgets >= 3.0; OpenGL/GLU for S57 performance; see `INSTALL` and the online dev manual linked in `INSTALL` for platform-specific packages.

- **Project-specific patterns & conventions**:
  - Language standard: C++17 (set with CMake). Visibility is often hidden by default (`-fvisibility=hidden`).
  - Singletons: many subsystems are singletons with `GetInstance()` (e.g., `PluginLoader::GetInstance()`, `PluginPaths::GetInstance()`).
  - Platform abstraction: `model/base_platform.h` and `g_BasePlatform` are used to query/install platform-specific paths.
  - Versioning: plugins and packages use `SemanticVersion` helpers (see `model/semantic_vers.*`). Prefer manipulating `SemanticVersion` when comparing plugin versions.
  - File-system utilities: repo includes `std_filesystem.h` and `ocpn::` helpers—use these for cross-platform file checks and path operations.

- **Plugin system notes (common tasks)**:
  - Loading logic: `PluginLoader` creates load-stamps, sets up `LD_LIBRARY_PATH`/`PATH`/`DYLD_LIBRARY_PATH` for helpers, and interprets plugin metadata.
  - Plugin installation metadata: look at `PluginHandler`, `plugin_cache`, and `catalog_handler` for managed plugin state and metadata flows.
  - Typical plugin dir locations are constructed in `PluginPaths` (`m_libdirs`, `m_bindirs`, `m_datadirs`). Use `OPENCPN_PLUGIN_DIRS` to override.

- **Where to look for examples**:
  - Plugin lifecycle & loader: `model/src/plugin_loader.cpp` and `model/src/plugin_handler.cpp`.
  - Plugin paths and platform rules: `model/src/plugin_paths.cpp`.
  - Build rules and options: `CMakeLists.txt` (root). Look at `build/` and `ci/` for platform toolchains and CI scripts.
  - Platform-specific code: `android/`, `buildlinux/`, `buildosx/`, `buildwin/`.

- **Common quick commands**:
  - Static analysis locally: enable `OCPN_ENABLE_CLANG_TIDY` in CMake.
  - Run unit/integration tests: `cmake -DOCPN_BUILD_TEST=ON ..` then `ctest -V`.
  - Search for plugin-related entry points: grep for `GetInstance()` in `model/` and `opencpn_plugin_` symbols for API levels.

- **Do / Don’t for PRs**:
  - Do follow existing naming and API versioning conventions when touching plugin-related code (preserve `opencpn_plugin_*` compatibility).
  - Do not change global build defaults (CMake options) without rationale—note that many downstream packaging assumptions depend on them.

If anything here is unclear or you'd like more detail for a specific subsystem (plugin lifecycle, build matrix, or CI), tell me which area and I'll expand the guidance or add small code examples.
