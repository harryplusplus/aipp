# Coding Agent Guidelines

## 일반 원칙

- **주석은 꼭 필요한 경우에만 작성한다.** 코드만으로 의도가 명확히 드러난다면 주석을 달지 않는다.
- 주석이 필요할 때는 **무엇(what)이 아니라 왜(why)와 어떻게(how)**를 설명한다.

---

## C++ 파일 작업 후
- `clang-format`을 실행하여 코드 스타일을 정리한다.

### 예제

**예제 1:** `src/core/processor.cc` 수정 후
```
# 수정 완료 → clang-format 실행
clang-format -i src/core/processor.cc
```

**예제 2:** `include/aipp/types.h` 수정 후
```
# 헤더 파일도 동일하게 적용
clang-format -i include/aipp/types.h
```

**예제 3:** 여러 파일을 동시에 수정한 경우
```
# 와일드카드로 한 번에 정리
clang-format -i src/**/*.cc include/**/*.h
```

---

## CMakeLists.txt 파일 작업 후
- `cmake-format`을 실행하여 파일을 정리한다.

### 예제

**예제 1:** 최상위 `CMakeLists.txt` 수정 후
```
cmake-format -i CMakeLists.txt
```

**예제 2:** 서브 디렉터리 `CMakeLists.txt` 수정 후
```
cmake-format -i src/CMakeLists.txt tests/CMakeLists.txt
```

**예제 3:** 새 타겟 추가 등 주요 변경 후
```
# 모든 CMakeLists.txt를 한 번에 정리
find . -name CMakeLists.txt -exec cmake-format -i {} +
```

## CMake 빌드

```
# Configure (초기 설정, build 디렉토리가 없거나 CMakeLists.txt 변경 시)
cmake -B build -S .

# Build
cmake --build build

# Clean & rebuild
cmake --build build --clean-first
```

> `CMakeLists.txt`를 변경한 경우 `cmake --build build`만으로 자동으로 reconfigure가 실행된다.
