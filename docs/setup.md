# TFCADIR Development Setup Guide

## Overview
This guide provides comprehensive instructions for setting up a development environment for TFCADIR contributions.

**Version:** 1.0
**Last Updated:** 2025-02-11
**For TFCADIR Version:** Phase 8.1 (Phases 1-7 Complete)

> **NOTE:** This documentation is currently located in `docs/.wip/phase-8-documentation/` as work-in-progress. After validation and review, it will be moved to the permanent documentation location.

## Table of Contents
- [Prerequisites](#prerequisites) - Required tools and libraries
- [Cloning the Repository](#cloning-the-repository) - Getting the source code
- [Building the Project](#building-the-project) - Compilation instructions
- [Development Workflow](#development-workflow) - Contribution process
- [Coding Standards](#coding-standards) - Code quality guidelines
- [Testing](#testing) - Running and writing tests
- [Debugging](#debugging) - Development debugging techniques
- [IDE Configuration](#ide-configuration) - Recommended IDE settings
- [Project-Specific Guidelines](#project-specific-guidelines) - TFCADIR conventions

---

## Prerequisites

### Required Tools

| Tool | Version | Purpose |
|------|---------|---------|
| **CMake** | 3.20+ | Build system |
| **C++ Compiler** | C++20 compatible | Modern C++ support |
| **Git** | Latest | Version control |
| **OpenGL** | 3.3+ | Graphics API |

### System Requirements

**Operating Systems:**
- Linux (Ubuntu 20.04+, Debian 11+)
- Windows (10+, MSVC 2019+)
- macOS (10.15+, Xcode, Clang)

**Minimum Specifications:**
- **RAM:** 4 GB recommended
- **Disk Space:** 2 GB for build + dependencies
- **Graphics:** OpenGL 3.3+ compatible GPU

---

## Cloning the Repository

### Clone with Submodules

```bash
# Clone repository with all submodules
git clone --recurse-submodules https://github.com/MaroonSkull/TFCADIR.git
cd TFCADIR
```

### Verify Submodules

Ensure all Git submodules are initialized:

```bash
# Check submodule status
git submodule status

# Update if needed
git submodule update --init --recursive
```

### Repository Structure

After cloning, you should see:

```
TFCADIR/
├── Dependencies/           # External libraries
│   ├── FSMConfig/        # State machine library
│   ├── glm/             # Math library
│   ├── glfwpp/           # Window/input wrapper
│   ├── imgui/           # GUI framework
│   └── spdlog/           # Logging
├── CMakeLists.txt          # Build configuration
├── Src/                    # Source code
│   ├── Controller/        # Input handling and FSM
│   ├── Model/            # Data structures
│   └── View/            # UI and rendering
└── build/                # Build output (generated)
```

---

## Building the Project

### First Build

```bash
# Configure CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build with all available cores
cmake --build build --parallel $(nproc)

# Run the application
cd build
./TFCADIR
```

### Build Types

| Target | Command | Description |
|--------|---------|-------------|
| **All** | `cmake --build build` | Build everything |
| **Debug** | `cmake --build build --config Debug` | Debug build with symbols |
| **Release** | `cmake --build build --config Release` | Optimized build |

### Build Output

The build directory contains:

```
build/
├── TFCADIR              # Executable
├── CMakeFiles/           # CMake configuration
├── Dependencies/            # Built dependencies
└── src/                   # Object files (from TFCADIR/Src)
```

---

## Development Workflow

### Branch Naming

Use descriptive branch names for features and fixes:

| Type | Format | Examples |
|-------|--------|----------|
| **Features** | `feature/short-description` | feature/drawing-tools, feature/snap-system |
| **Bug Fixes** | `bugfix/short-description` | bugfix/crash-on-startup, bugfix/memory-leak |
| **Refactoring** | `refactor/short-description` | refactor/optimize-rendering, refactor/clean-fsm |
| **Documentation** | `docs/short-description` | docs/update-api-reference, docs/user-guide |

### Commit Guidelines

1. **Atomic Commits** - One logical change per commit
2. **Clear Messages** - Describe what and why
3. **Reference Issues** - Link related issues (e.g., `Closes #42`)
4. **Follow Format** - Use conventional commit types

**Commit Message Format:**

```
<type>: <short-description>

<detailed-description (optional)>

<references (optional)>
```

**Types:**
- `feat` - New feature
- `fix` - Bug fix
- `refactor` - Code refactoring
- `docs` - Documentation changes
- `test` - Test additions
- `chore` - Maintenance tasks

**Examples:**

```
feat: add support for nested states

Implemented hierarchical state structure with inheritance.
Added tests in test_state_machine.cpp.

Closes #42
```

```
fix: fix memory leak in ConfigParser

The problem was missing YAML::Node cleanup.
Now using smart pointer.
```

### Pull Request Process

For larger contributions:

1. **Fork** repository
2. **Create** a feature branch
3. **Implement** your changes
4. **Test** thoroughly
5. **Submit** pull request to `MaroonSkull/TFCADIR`

---

## Coding Standards

### Code Style

Follow project conventions for consistent code style:

**Formatting:**
- Use 4 spaces for indentation
- Maximum line length: 120 characters
- Use clang-format for automatic formatting

**Naming Conventions:**
- **Classes:** PascalCase (e.g., `CameraController`, `ToolManager`)
- **Functions:** camelCase (e.g., `getActiveTool()`, `executeCommand()`)
- **Members:** camelCase with trailing underscore (e.g., `fsmAdapter_`, `cameraController_`)
- **Private Members:** trailing underscore (e.g., `nextId_`, `vertexCount_`)

### Documentation Style

Use Doxygen-style comments:

```cpp
/**
 * @brief Brief description of function
 * @param paramName Description of parameter
 * @return Description of return value
 *
 * Detailed description if needed
 */
```

**Comment Placement:**
- Above the element being documented
- Use `///` for single-line comments
- Use `/**` for multi-line comments

### Best Practices

1. **RAII** - Use smart pointers and RAII principles
2. **Const Correctness** - Mark methods `const` when appropriate
3. **Modern C++** - Use C++20 features (auto, constexpr, range-based for)
4. **Error Handling** - Use exceptions for error conditions
5. **Include Guards** - Use proper include guards

---

## Testing

### Running Tests

```bash
# Build tests
cmake --build build --target test

# Run all tests
cd build
ctest --output-on-failure
```

### Test Organization

Tests are organized by component:

```
Src/Controller/Tests/
Src/Model/Tests/
Src/View/Tests/
```

### Writing Tests

Follow naming convention: `test_<component>_<feature>.cpp`

**Example:**
```cpp
// test_tool_manager.cpp
TEST(ToolManager, ActivateTool) {
    EXPECT_EQ(tool->getActiveTool(), "Line3D");
    // Test implementation
}
```

---

## Debugging

### Development Debugging

**Debug Build:**
```bash
cmake --build build --config Debug
```

**Logging:**

TFCADIR uses spdlog for logging. Enable debug logging in settings or code:

```cpp
// Enable debug logging
spdlog::set_level(spdlog::level::debug);
```

### Common Issues and Solutions

#### Issue: Build Errors

**Symptoms:**
- CMake configuration fails
- Compilation errors
- Linker errors

**Solutions:**
1. Check CMake version (3.20+)
2. Ensure all submodules are initialized
3. Verify OpenGL and GLM are available
4. Clean build directory: `rm -rf build && cmake -B build`

#### Issue: Runtime Crashes

**Symptoms:**
- Application crashes on startup
- Crashes during specific operations

**Solutions:**
1. Run with debugger (GDB, LLDB)
2. Check for null pointer dereferences
3. Review recent changes
4. Enable debug logging for more information

#### Issue: OpenGL Errors

**Symptoms:**
- Rendering artifacts or missing geometry
- GLSL compilation errors

**Solutions:**
1. Check OpenGL context is properly initialized
2. Verify shader compilation
3. Check for OpenGL errors using `glGetError()`
4. Review graphics driver compatibility

---

## IDE Configuration

### Visual Studio Code

**Recommended Extensions:**
- C/C++ Extension for VS Code
- CMake Tools for Visual Studio

**Settings (.vscode/settings.json):**

```json
{
    "C_Cpp.default.cppStandard": "c++20",
    "C_Cpp.default.compilerPath": "/usr/bin/clang++",
    "files.associations": {
        "*.cpp": "cpp",
        "*.hpp": "cpp"
    },
    "cmake.configureOnOpen": true
}
```

### CLion

**Settings:**
- Enable Clang-Tidy
- Enable Clang-Format
- Set code style to Google or LLVM

---

## Project-Specific Guidelines

### TFCADIR Conventions

1. **MVC Pattern** - Follow Model-View-Controller separation
2. **FSM-Driven** - All UI state managed by FSM
3. **Stateless Managers** - Query state from `view::UIFSMAdapter`
4. **Command Pattern** - All actions implement `Controller::ICommand` interface

**Architecture Notes:**

**Stateless Coordinator Pattern:**

TFCADIR uses a stateless coordinator pattern for UI management:
- **`view::UIFSMAdapter`** serves as the single source of truth for all UI state
- **NavigationManager**, **SelectionManager**, and other managers are stateless - they query UIFSMAdapter for state
- This pattern ensures:
  - Consistent state across all components
  - No duplicate state management
  - Clear separation of concerns

**Component Dependencies:**

When adding new features, maintain loose coupling:
- **Model ↔ View** - Via UIFSMAdapter callbacks
- **Controller → Model** - Via `Model::FigureCreator` factory
- **View → Controller** - Via `Controller::CameraController` reference

---

## Related Documentation

- [Architecture Documentation](../architecture.md) - System architecture details
- [API Reference](../api_reference.md) - Component API documentation
- [User Manual](../user_manual.md) - End-user documentation

---

## Getting Help

If you encounter issues not covered in this guide:

1. **Check Documentation** - Review relevant sections above
2. **GitHub Issues** - Search or report issues
3. **Community** - Ask questions in discussions

---

**Document Information**

- **Version:** 1.0
- **Last Updated:** 2025-02-11
- **For TFCADIR Version:** Phase 8.1 (Phases 1-7 Complete)

**License:** See LICENSE file in project repository

For complete development documentation, visit the [TFCADIR GitHub repository](https://github.com/MaroonSkull/TFCADIR).
