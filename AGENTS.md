# Coding Agent Guidelines

## 문서 가이드라인

| 문서 | 대상 | 내용 |
|---|---|---|
| `README.md` | **사용자** | 프로젝트 개요, 설치, 사용법 |
| `AGENTS.md` | **AI 에이전트** | 개발 규칙, 도구 사용법, 실험 방법론 |

- AI 에이전트는 **먼저 `README.md`를 읽고**, 그다음 이 파일을 따라 작업한다.
- 개발 상세는 이 파일에만 기록하고, `README.md`는 사용자 수준으로 유지한다.
- 이 파일은 **AI 에이전트의 행동 지침**이다. 모든 내용은 "무엇을 할지", "왜 해야 하는지", "어떻게 할지"로 구성한다.
- 정보를 나열하지 말고, **파일 위치**를 명시하여 에이전트가 직접 읽게 한다.

---

## 일반 원칙

- **주석은 꼭 필요한 경우에만.** 코드만으로 의도가 명확하면 달지 않는다. 주석이 필요하면 **무엇(what)이 아니라 왜(why)와 어떻게(how)**를 설명한다.
- **Google C++ 스타일 가이드**(`external/styleguide/cppguide.html`)를 준수한다. 소스는 `.cc`, 헤더는 `.h`만 사용한다.

## rg 사용법 — 반드시 숙지할 것

`rg`는 **Rust 정규식**을 사용한다. **`|`는 OR 연산자이므로 절대 escape하지 않는다.**

```bash
# OR 검색
rg "foo|bar"          # foo 또는 bar 검색 (정상)
rg "foo\|bar"         # literal "foo|bar" 검색 — 의도한 OR가 아님! (오류)
rg "foo[|]bar"        # literal "foo|bar" 검색 (정상, 문자 클래스 사용)
```

| 패턴 | 결과 |
|---|---|
| `"foo|bar"` | `foo` 또는 `bar` (OR) |
| `"foo\|bar"` | literal `foo|bar` (의도한 OR가 아님) |
| `"foo[|]bar"` | literal `foo|bar` (권장) |

**기억할 것:** `rg`에서는 `|`를 절대 escape하지 않는다. Escape하면 literal pipe가 된다.

## 빌드 (CMakePresets)

`CMakePresets.json`에 preset이 정의되어 있다. `--preset` 플래그로 `-G Ninja`, `-DBUILD_TESTING=ON`, `-DCMAKE_BUILD_TYPE=Debug` 등을 대체한다.

```bash
# Configure (debug, 테스트 포함)
cmake --preset debug

# 빌드
cmake --build --preset debug

# Clean rebuild
cmake --build --preset debug --clean-first

# Configure (release, 테스트 제외)
cmake --preset release
cmake --build --preset release
```

`CMakeLists.txt`를 변경해도 `cmake --build --preset debug`만으로 자동 reconfigure된다.

## C++ 코드 변경 후

변경이 끝나면 반드시 아래 세 단계를 순서대로 실행한다.

```bash
# 1. 포맷팅
clang-format -i src/main.cc

# 2. 스타일 검사
cpplint src/main.cc

# 3. 정적 분석
clang-tidy -p build src/main.cc
clang-tidy -p build -fix src/main.cc   # 자동 수정
```

와일드카드로 한 번에 처리:
```bash
clang-format -i src/**/*.cc src/**/*.h
find src -name '*.cc' -o -name '*.h' | xargs clang-tidy -p build
```

`clang-tidy -p build`는 `build/compile_commands.json`을 읽어 include 경로와 컴파일 플래그를 파악한다. `CMAKE_EXPORT_COMPILE_COMMANDS ON`이 설정되어 있어 자동 생성된다.

## CMakeLists.txt 변경 후

```bash
cmake-format -i CMakeLists.txt
```

## 테스트 (GTest + CTest)

**테스트 실행 (전체):**
```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

**특정 테스트만 실행:**
```bash
./build/aipp_test --gtest_filter='Sanity.*'
```

> `--gtest_filter='Sanity.*'`의 싱글쿼트는 `*`가 쉘에서 확장되지 않도록 보호한다. 반드시 붙일 것.

**추가:** `src/` 아래에 `.cc` 파일로 새 테스트를 만들고, `CMakeLists.txt`에 `add_executable` / `add_test()`를 등록한다.

---

## 실험 및 검증 방법론

### 검증 사이클

```
코드 수정 → 포맷팅/분석 → 빌드 → 실행 테스트 → 결과 확인
```

수정 후 반드시 포맷팅 → 빌드 → 실행까지 완료한다. "될 것이다"라고 추정하지 않는다.

### TUI 테스트 (tmux detached)

```bash
tmux new-session -d -s tui_test
tmux send-keys -t tui_test './build/aipp' Enter
tmux send-keys -t tui_test 'Hello'
tmux send-keys -t tui_test $'\x1B[27;2;13~'   # Shift+Enter (CSI u)
tmux capture-pane -t tui_test -p
tmux send-keys -t tui_test C-c
tmux kill-session -t tui_test
```

### 문제 발생 시 — 최소 단위로 분할 (이분법)

1. 가장 단순한 입력/출력부터 동작 확인
2. 문제가 되는 부분을 절반으로 나눠 어느 쪽이 문제인지 좁힘
3. 원인으로 의심되는 부분을 격리해서 단독 테스트

**한 번에 하나씩만 변경**하고 매번 빌드 & 실행한다.

### 키 이벤트 확인

터미널 환경마다 특수 키가 보내는 시퀀스가 다르다. **추정하지 말고 직접 측정**한다.

```bash
# 키 입력의 원시 시퀀스 확인
od -c                     # 입력 후 Ctrl+D로 종료
# 또는
cat -v                    # 입력 후 Ctrl+D로 종료
```

### 라이브러리 동작 확인

소스 코드를 직접 읽는다. 추측 금지.

```bash
# 의존성 라이브러리 소스 확인 (build/_deps/)
cat build/_deps/*-src/src/*.cpp
```
