# AI Agent Guidelines

## 문서 가이드라인

| 문서 | 대상 | 내용 |
|---|---|---|
| `README.md` | **사용자** | 프로젝트 개요, 설치, 사용법 |
| `AGENTS.md` | **AI 에이전트** | 개발 규칙, 도구 사용법, 실험 방법론 |

- AI 에이전트는 **먼저 `README.md`를 읽고**, 그다음 이 파일을 따라 작업한다.
- 개발 상세는 이 파일에만 기록하고, `README.md`는 사용자 수준으로 유지한다.

## 일반 원칙

- **주석은 왜(why)만.** 주석을 달기 전에 "이걸 코드 자체로 표현할 수 없나?"를 먼저 고민한다. 주석에는 **무엇(what)과 어떻게(how)를 절대 적지 않는다** — 그건 코드가 할 일이다. 오직 **왜 이렇게 했고, 왜 다른 방법은 안 되는지(why)**만 적는다.
  - ❌ `// count를 증가시킴` (what — 코드가 이미 말함)
  - ❌ `// 리소스 해제` (what)
  - ❌ `// --- Positive cases ---` (조직용 레이블 — 구조 자체로 드러내라)
  - ✅ `// rename()이 atomic을 보장하지 않아서 copy + unlink로 fallback` (why)
  - ✅ `// 임시 파일이 남으면 디스크가 찰 수 있어서 반드시 정리` (why)
  - ✅ `// 이 플랫폼에서는 O_DIRECT가 정합성을 보장하지 않음` (why)
- **Google C++ 스타일 가이드**(`external/styleguide/cppguide.html`)를 준수한다.
  코드 작성 전에 다음 세 섹션을 먼저 읽는다:
  1. Naming — 타입/함수/변수/상수/데이터멤버 명명 규칙 (`read ... -offset 4347 -limit 380`)
  2. Names and Order of Includes — include 순서 (`read ... -offset 441 -limit 124`)
  3. Declaration Order — 클래스 선언 순서 (`read ... -offset 1807 -limit 42`)
- **일관성**: 모든 작업 흐름 단계, 제안, 코드는 이 원칙들과 모순되어서는 안 된다.
  새 단계를 추가하거나 기존 단계를 수정할 때는 먼저 원칙들과의 일관성을 검증한다.

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

> 아래 모든 단계는 [일반 원칙](#일반-원칙)을 전제로 하며, 이와 모순되는 단계를
> 추가하거나 수정할 수 없다.

### 0. 원칙 검증
수행하려는 작업(코드 작성, 제안, 문서화 등)이 [일반 원칙](#일반-원칙)을 위반하지 않는지 먼저 확인한다.
위반 시 다른 접근을 찾거나, 원칙을 수정해야 한다면 그 이유를 명시한다.

### 1. 구성 — 소스 추가/삭제 또는 CMakeLists.txt 변경 시

> clang-tidy와 빌드가 `compile_commands.json`에 의존하므로 구성이 선행되어야 한다.

```bash
cmake --preset debug
```

### 2. clang-format — 포맷팅

```bash
# 특정 파일
clang-format -i <path/to/file.cc> <path/to/file.h>

# 일괄
clang-format -i src/*.cc src/*.h
```

### 3. clang-tidy — 정적 분석 (autofix)

```bash
# 특정 파일
clang-tidy -p build -fix <path/to/file.cc> <path/to/file.h>

# 일괄
clang-tidy -p build -fix src/*.cc src/*.h
```

### 4. cpplint — Google 스타일 검사

```bash
# 특정 파일
cpplint <path/to/file.cc> <path/to/file.h>

# 일괄
cpplint --recursive src/
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
