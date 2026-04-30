# AI++

An AI agent harness TUI application written in C++.

## Development

```bash
git clone <repo-url>
cd aipp
git submodule update --init --recursive
```

**Required CLI tools:**

- `cmake` — build system
- `ninja` — build tool
- `cmake-format` — CMakeLists.txt formatter
- `clang-format` — C++ code formatter

**Tests (GTest + CTest):**

```bash
# 테스트 포함 빌드
cmake -B build -S . -DBUILD_TESTING=ON
cmake --build build

# 테스트 실행
ctest --test-dir build -V
```

> `BUILD_TESTING=OFF`(기본값)일 때는 GTest를 Fetch하지 않아 빌드가 가볍다.
