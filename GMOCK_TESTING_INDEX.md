# V2X Hub Plugin Unit Testing Framework - Complete Documentation Index

## 📍 Start Here

**New to mocking PluginClient?** Start with one of these:

1. **[HOW_TO_MOCK_PLUGINCLIENT.md](./HOW_TO_MOCK_PLUGINCLIENT.md)** ⭐ **BEST FIRST READ**
   - 5-minute overview of the entire concept
   - Quick start guide
   - Common patterns
   - Key takeaways
   - **Read this first if you have limited time**

2. **[GMOCK_TESTING_OVERVIEW.md](./GMOCK_TESTING_OVERVIEW.md)** 
   - Complete overview of the framework
   - What's included
   - How it works
   - Architecture and file organization
   - Next steps

3. **[tests/unit/QUICK_REFERENCE.md](./tests/unit/QUICK_REFERENCE.md)**
   - Syntax cheat sheet
   - Copy-paste templates
   - Common patterns
   - Build commands
   - **Perfect for referencing while coding**

## 📚 Full Documentation

### Complete Guides
- **[docs/GMock_PluginClient_Unit_Testing_Guide.md](./docs/GMock_PluginClient_Unit_Testing_Guide.md)**
  - 50+ pages of comprehensive coverage
  - Step-by-step examples
  - Advanced techniques
  - Best practices
  - Complete architecture explanation
  - **Read this for deep understanding**

- **[tests/unit/README.md](./tests/unit/README.md)**
  - User guide for the framework
  - How to create tests
  - Testing patterns
  - Running tests
  - **Reference while creating your tests**

### Problem Solving
- **[tests/unit/TROUBLESHOOTING.md](./tests/unit/TROUBLESHOOTING.md)**
  - 30+ common issues and solutions
  - Build problems
  - Runtime errors
  - GMock issues
  - Debugging tips
  - **Consult when something breaks**

## 💻 Code Examples

### Basic Examples
- **[tests/unit/ExamplePluginTest.cpp](./tests/unit/ExamplePluginTest.cpp)**
  - Simple plugin testing
  - Configuration testing
  - State transitions
  - Message handling
  - **Start with these examples**

### Advanced Examples
- **[tests/unit/plugins/RealPluginExampleTest.cpp](./tests/unit/plugins/RealPluginExampleTest.cpp)**
  - Realistic plugin implementation
  - Complete lifecycle testing
  - Integration patterns
  - Stress testing
  - **Study these for advanced patterns**

## 🔧 Implementation Files

### Mock Framework
- **[tests/unit/mocks/MockPluginClient.h](./tests/unit/mocks/MockPluginClient.h)** ⭐ MOST IMPORTANT
  - Complete mock implementation
  - Well-documented
  - Ready to use
  - Contains matchers and helpers
  
- **[tests/unit/mocks/MockPluginClient.cpp](./tests/unit/mocks/MockPluginClient.cpp)**
  - Implementation details
  - Constructor/destructor

### Build Configuration
- **[tests/unit/CMakeLists.txt](./tests/unit/CMakeLists.txt)**
  - Complete CMake configuration
  - Ready to use as template
  - Includes coverage options
  - Build instructions

## 🎯 How to Use This Framework

### Scenario 1: Quick Start (5 minutes)
1. Read: `HOW_TO_MOCK_PLUGINCLIENT.md`
2. Copy: `tests/unit/CMakeLists.txt` template
3. Review: `tests/unit/QUICK_REFERENCE.md`
4. Create: Your first test

### Scenario 2: Learn Properly (30 minutes)
1. Read: `GMOCK_TESTING_OVERVIEW.md`
2. Study: `tests/unit/ExamplePluginTest.cpp`
3. Review: `docs/GMock_PluginClient_Unit_Testing_Guide.md` (Introduction section)
4. Practice: Create a test for a simple plugin

### Scenario 3: Deep Dive (2+ hours)
1. Read: `docs/GMock_PluginClient_Unit_Testing_Guide.md` (entire)
2. Study: Both example files in detail
3. Review: `tests/unit/README.md` completely
4. Practice: Create comprehensive tests for multiple plugins

### Scenario 4: Troubleshooting (when stuck)
1. Check: `tests/unit/TROUBLESHOOTING.md` for your issue
2. Review: Related section in `docs/GMock_PluginClient_Unit_Testing_Guide.md`
3. Consult: `tests/unit/QUICK_REFERENCE.md` for syntax
4. Study: Related example in test files

## 📋 Documentation Structure

### By Purpose

#### "What is this?" / "How does it work?"
- `HOW_TO_MOCK_PLUGINCLIENT.md` - Beginner-friendly overview
- `GMOCK_TESTING_OVERVIEW.md` - Complete framework overview
- `docs/GMock_PluginClient_Unit_Testing_Guide.md` - Deep technical explanation

#### "How do I use it?"
- `tests/unit/QUICK_REFERENCE.md` - Syntax and patterns
- `tests/unit/README.md` - Workflow and usage
- `tests/unit/ExamplePluginTest.cpp` - Working examples

#### "How do I build it?"
- `tests/unit/CMakeLists.txt` - Build configuration
- `tests/unit/README.md` - Building section
- `tests/unit/QUICK_REFERENCE.md` - Build commands

#### "What went wrong?"
- `tests/unit/TROUBLESHOOTING.md` - Problem solving
- `docs/GMock_PluginClient_Unit_Testing_Guide.md` - Common issues section

#### "Show me examples"
- `tests/unit/ExamplePluginTest.cpp` - Basic examples
- `tests/unit/plugins/RealPluginExampleTest.cpp` - Advanced examples
- `docs/GMock_PluginClient_Unit_Testing_Guide.md` - Multiple examples throughout

### By Audience

#### Visual Learners
- Study the example files first
- Look at diagrams in main guides
- Follow step-by-step tutorials

#### Reading Learners
- Start with `HOW_TO_MOCK_PLUGINCLIENT.md`
- Read `docs/GMock_PluginClient_Unit_Testing_Guide.md` completely
- Reference `tests/unit/README.md` as needed

#### Hands-On Learners
- Look at examples first
- Try to create your own test
- Refer to `QUICK_REFERENCE.md` while coding
- Use `TROUBLESHOOTING.md` when stuck

#### Copy-Paste Learners
- Use `tests/unit/QUICK_REFERENCE.md` for templates
- Reference `ExamplePluginTest.cpp` for patterns
- Modify `CMakeLists.txt` from provided template

## 🔍 Topic Index

### Core Concepts
- **Mocking**: `HOW_TO_MOCK_PLUGINCLIENT.md` → "The Solution"
- **Virtual Methods**: `docs/GMock_PluginClient_Unit_Testing_Guide.md` → Step 1
- **Architecture**: `GMOCK_TESTING_OVERVIEW.md` → "Architecture"
- **Benefits**: `GMOCK_TESTING_OVERVIEW.md` → "Benefits"

### Implementation
- **Creating Mock**: `docs/GMock_PluginClient_Unit_Testing_Guide.md` → Step 2-3
- **Test Fixtures**: `tests/unit/README.md` → "Creating Tests"
- **Building**: `tests/unit/CMakeLists.txt` and README
- **Running**: `tests/unit/QUICK_REFERENCE.md` → "Test Execution"

### Patterns
- **Configuration Testing**: `tests/unit/ExamplePluginTest.cpp` → ConfigurationTests
- **State Management**: `tests/unit/ExamplePluginTest.cpp` → StateChangeTests
- **Message Handling**: `tests/unit/ExamplePluginTest.cpp` → MessageHandlingTests
- **Error Handling**: `tests/unit/ExamplePluginTest.cpp` → ErrorHandlingTests
- **Integration**: `tests/unit/plugins/RealPluginExampleTest.cpp` → IntegrationTests

### GMock
- **EXPECT_CALL**: `tests/unit/QUICK_REFERENCE.md` → "GMock Expectations"
- **Matchers**: `docs/GMock_PluginClient_Unit_Testing_Guide.md` → Matchers section
- **Return Values**: `tests/unit/QUICK_REFERENCE.md` → "Return Values"
- **Callbacks**: `docs/GMock_PluginClient_Unit_Testing_Guide.md` → Advanced Techniques

### Troubleshooting
- **Build Errors**: `tests/unit/TROUBLESHOOTING.md` → "Build Issues"
- **Runtime Errors**: `tests/unit/TROUBLESHOOTING.md` → "Runtime Issues"
- **Mock Problems**: `tests/unit/TROUBLESHOOTING.md` → "Mock-Specific Issues"
- **Debugging**: `tests/unit/TROUBLESHOOTING.md` → "Debugging Tips"

## 📦 Quick Reference by Task

| Task | Where to Look |
|------|----------------|
| Understand the concept | `HOW_TO_MOCK_PLUGINCLIENT.md` |
| Build and run first test | `tests/unit/QUICK_REFERENCE.md` |
| Learn GMock syntax | `tests/unit/QUICK_REFERENCE.md` + `docs/GMock_PluginClient_Unit_Testing_Guide.md` |
| See working examples | `tests/unit/ExamplePluginTest.cpp` |
| Test configuration | `tests/unit/ExamplePluginTest.cpp` + `docs/GMock_PluginClient_Unit_Testing_Guide.md` |
| Test messages | `tests/unit/plugins/RealPluginExampleTest.cpp` + examples |
| Test state | `tests/unit/ExamplePluginTest.cpp` |
| Create CMakeLists.txt | `tests/unit/CMakeLists.txt` (copy this) |
| Mock a specific method | `tests/unit/QUICK_REFERENCE.md` → "Key Concepts" |
| Capture arguments | `docs/GMock_PluginClient_Unit_Testing_Guide.md` → "Advanced Techniques" |
| Fix build error | `tests/unit/TROUBLESHOOTING.md` → "Build Issues" |
| Fix runtime error | `tests/unit/TROUBLESHOOTING.md` → "Runtime Issues" |
| Test doesn't compile | `tests/unit/TROUBLESHOOTING.md` → "Mock-Specific Issues" |

## 🚀 Getting Started Paths

### Path 1: Impatient Developer (15 min)
```
1. Read: HOW_TO_MOCK_PLUGINCLIENT.md (5 min)
2. Copy: tests/unit/CMakeLists.txt
3. Copy: tests/unit/ExamplePluginTest.cpp
4. Modify for your plugin and build
```

### Path 2: Pragmatic Developer (45 min)
```
1. Read: HOW_TO_MOCK_PLUGINCLIENT.md (5 min)
2. Read: GMOCK_TESTING_OVERVIEW.md (10 min)
3. Read: tests/unit/QUICK_REFERENCE.md (10 min)
4. Study: tests/unit/ExamplePluginTest.cpp (15 min)
5. Create your first test (5 min)
```

### Path 3: Thorough Developer (2+ hours)
```
1. Read: GMOCK_TESTING_OVERVIEW.md (15 min)
2. Read: docs/GMock_PluginClient_Unit_Testing_Guide.md (60 min)
3. Study: All example files (30 min)
4. Read: tests/unit/README.md (15 min)
5. Create comprehensive tests (30+ min)
```

### Path 4: Debugging Developer (30 min)
```
1. Check: tests/unit/TROUBLESHOOTING.md for your issue
2. Read: Relevant section in main guide
3. Review: Related example
4. Apply: Solution to your code
```

## 📞 Need Help?

### For Concepts
→ Read `HOW_TO_MOCK_PLUGINCLIENT.md` or `GMOCK_TESTING_OVERVIEW.md`

### For Syntax
→ Check `tests/unit/QUICK_REFERENCE.md`

### For Examples
→ Study `tests/unit/ExamplePluginTest.cpp` or `tests/unit/plugins/RealPluginExampleTest.cpp`

### For Deep Understanding
→ Read `docs/GMock_PluginClient_Unit_Testing_Guide.md`

### For Problems
→ Check `tests/unit/TROUBLESHOOTING.md`

### For Workflow
→ See `tests/unit/README.md`

## 📊 Document Overview

```
HOW_TO_MOCK_PLUGINCLIENT.md (4 pages) ⭐ START HERE
    ↓
GMOCK_TESTING_OVERVIEW.md (10 pages)
    ↓
docs/GMock_PluginClient_Unit_Testing_Guide.md (50+ pages)
    ↓
tests/unit/README.md (20+ pages) + tests/unit/QUICK_REFERENCE.md (10 pages)
    ↓
Code Examples: ExamplePluginTest.cpp + RealPluginExampleTest.cpp
    ↓
tests/unit/TROUBLESHOOTING.md (15+ pages)
```

## ✨ Key Files Summary

| File | Purpose | Pages | Read Time |
|------|---------|-------|-----------|
| `HOW_TO_MOCK_PLUGINCLIENT.md` | Overview | 4 | 5 min |
| `GMOCK_TESTING_OVERVIEW.md` | Complete overview | 10 | 15 min |
| `docs/GMock_PluginClient_Unit_Testing_Guide.md` | Comprehensive guide | 50+ | 60 min |
| `tests/unit/README.md` | Usage guide | 20+ | 20 min |
| `tests/unit/QUICK_REFERENCE.md` | Cheat sheet | 10 | 10 min |
| `tests/unit/TROUBLESHOOTING.md` | Problem solver | 15+ | As needed |
| `tests/unit/ExamplePluginTest.cpp` | Basic examples | - | 10 min |
| `tests/unit/plugins/RealPluginExampleTest.cpp` | Advanced examples | - | 15 min |
| `tests/unit/mocks/MockPluginClient.h` | Mock implementation | - | 5 min |
| `tests/unit/CMakeLists.txt` | Build config | - | Reference |

## 🎓 Recommended Reading Order

1. **First Time?** → `HOW_TO_MOCK_PLUGINCLIENT.md`
2. **Want Overview?** → `GMOCK_TESTING_OVERVIEW.md`
3. **Ready to Code?** → `tests/unit/QUICK_REFERENCE.md` + `ExamplePluginTest.cpp`
4. **Need Details?** → `docs/GMock_PluginClient_Unit_Testing_Guide.md`
5. **Stuck?** → `tests/unit/TROUBLESHOOTING.md`

---

**Last Updated**: November 2025  
**Framework Version**: 1.0  
**Status**: Production Ready ✅

**Quick Links:**
- [Main Overview](./HOW_TO_MOCK_PLUGINCLIENT.md)
- [Framework Overview](./GMOCK_TESTING_OVERVIEW.md)
- [Quick Reference](./tests/unit/QUICK_REFERENCE.md)
- [Full Guide](./docs/GMock_PluginClient_Unit_Testing_Guide.md)
- [Troubleshooting](./tests/unit/TROUBLESHOOTING.md)
