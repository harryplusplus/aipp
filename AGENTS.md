# Coding Agent Guidelines

> **시작하기 전에 `README.md`를 먼저 읽어서 프로젝트 개요와 개발 환경을 숙지하시오.**

## 일반 원칙

- **주석은 꼭 필요한 경우에만 작성한다.** 코드만으로 의도가 명확히 드러난다면 주석을 달지 않는다.
- 주석이 필요할 때는 **무엇(what)이 아니라 왜(why)와 어떻게(how)**를 설명한다.
- **Google C++ 스타일 가이드**(`external/styleguide/cppguide.html`)를 준수한다.
- **한국어를 포함한 모든 UTF-8 입력을 지원한다.** FTXUI가 UTF-8을 정상 처리하므로, IME 입력이 안 된다면 터미널/환경 문제로 간주하고 이슈로 남긴다.

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

---

## 실험 및 검증 방법론

### 1. TUI 프로그램은 tmux detached 세션에서 테스트한다

```bash
# 세션 생성
tmux new-session -d -s tui_test

# 프로그램 실행
tmux send-keys -t tui_test './build/aipp' Enter

# 키 입력 전송 (영어)
tmux send-keys -t tui_test 'Hello'

# 특수 키: Shift+Enter (CSI u 시퀀스는 한 번에 전송)
tmux send-keys -t tui_test $'\x1B[27;2;13~'

# 화면 캡처
tmux capture-pane -t tui_test -p 2>/dev/null

# 종료 후 정리
tmux send-keys -t tui_test C-c   # Ctrl+C로 종료
tmux kill-session -t tui_test     # 세션 제거
```

### 2. 문제 발생 시 최소 단위로 쪼개서 테스트한다 (이분법)

1. **컴포넌트 단독 테스트**: Input 단독으로 먼저 동작 확인
   ```cpp
   screen.Loop(input);  // Container 없이 Input만
   ```
2. **Container 추가 시 문제**: 자식 순서 바꾸기, `SetActiveChild`로 포커스 강제 지정
3. **Event 처리 문제**: CatchEvent 제거 후 순수 Container로 동작 확인

반드시 **한 번에 하나씩만 변경**하고 매번 빌드 & 실행한다.

### 3. 키 이벤트 분석은 print_key_press를 사용한다

```bash
# 빌드 (FTXUI_BUILD_EXAMPLES=ON 필요)
cmake -B build -S . -DFTXUI_BUILD_EXAMPLES=ON
cmake --build build --target ftxui_example_print_key_press

# 실행 후 키 입력 → 좌측: ASCII 코드, 우측: 이벤트 이름
./build/_deps/ftxui-build/examples/component/ftxui_example_print_key_press
```

터미널 환경마다 특수 키(Shift+Enter 등)의 이벤트 코드가 다르므로, **추정하지 말고 반드시 직접 측정**한다.

### 4. 라이브러리 동작 확인은 소스 코드를 직접 읽는다

```bash
# Input 컴포넌트의 Enter 처리 확인
rg "HandleReturn|multiline" build/_deps/ftxui-src/src/ftxui/component/input.cpp

# 키 입력 파싱 확인
cat build/_deps/ftxui-src/src/ftxui/component/terminal_input_parser.cpp
```

상상이나 추측으로 동작을 예측하지 말고, 실제 코드를 읽어서 확인한다.

### 5. 검증 사이클

```
코드 수정 → clang-format / cmake-format → 빌드 → tmux 테스트 → 결과 확인
```

수정 후 반드시 포맷팅 → 빌드 → 실행 테스트까지 완료해야 한다. 화면 출력만 보고 "될 것이다"라고 추정하지 않는다.
