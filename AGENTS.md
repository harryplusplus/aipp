# Coding Agent Guidelines

## C++ 파일 작업 후
- `clang-format`을 실행하여 코드 스타일을 정리한다.

### Few-shot 예제

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

### Few-shot 예제

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
