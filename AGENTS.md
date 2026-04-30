# AI Agent Guidelines

## 문서 가이드라인

| 문서 | 대상 | 내용 |
|---|---|---|
| `README.md` | **사용자** | 프로젝트 개요, 설치, 사용법 |
| `AGENTS.md` | **AI 에이전트** | 개발 규칙, 도구 사용법, 실험 방법론 |

- AI 에이전트는 **먼저 `README.md`를 읽고**, 그다음 이 파일을 따라 작업한다.
- 개발 상세는 이 파일에만 기록하고, `README.md`는 사용자 수준으로 유지한다.

## 일반 원칙

- **주석은 꼭 필요한 경우에만.** 코드만으로 의도가 명확하면 달지 않는다. 주석이 필요하면 **무엇(what)이 아니라 왜(why)와 어떻게(how)**를 설명한다.
- **Google C++ 스타일 가이드**(`external/styleguide/cppguide.html`)를 준수한다.

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

## 작업 흐름

### 1. 구성 — 소스 추가/삭제 또는 CMakeLists.txt 변경 시

> clang-tidy와 빌드가 `compile_commands.json`에 의존하므로 구성이 선행되어야 한다.

```bash
cmake --preset debug
```

### 2. clang-tidy — 정적 분석 (autofix)

```bash
# 특정 파일
clang-tidy -p build -fix <path/to/file.cc> <path/to/file.h>

# 일괄
clang-tidy -p build -fix src/*.cc src/*.h
```

### 3. cpplint — Google 스타일 검사

```bash
# 특정 파일
cpplint <path/to/file.cc> <path/to/file.h>

# 일괄
cpplint --recursive src/
```

### 4. clang-format — 포맷팅

```bash
# 특정 파일
clang-format -i <path/to/file.cc> <path/to/file.h>

# 일괄
clang-format -i src/*.cc src/*.h
```

### 5. 빌드

```bash
cmake --build --preset debug
```

> 1~4단계 중 어느 단계에서든 파일이 수정되거나 실패하면 1단계로 돌아간다.
> 모두 통과할 때까지 반복한다.

### 5. 회귀 테스트

위 1~4단계가 모두 통과한 후에만 실행한다.

```bash
ctest --preset debug
```

### 6. 수동 TUI 테스트

tmux detached 세션에서 동작을 확인한다.
작업 내용에 맞게 키 입력과 시점을 변경하여 실행한다.

```bash
tmux new-session -d -s tui_test
tmux send-keys -t tui_test './build/aipp' Enter
tmux send-keys -t tui_test 'Hello'
tmux send-keys -t tui_test $'\x1B[27;2;13~'   # Shift+Enter (CSI u)
tmux capture-pane -t tui_test -p
tmux send-keys -t tui_test C-c
tmux kill-session -t tui_test
```
