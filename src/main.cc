#include <locale.h>
#include <ncurses.h>

#include <iostream>
#include <print>
#include <string>
#include <utility>
#include <vector>

#include "src/defer.h"
#include "src/resource.h"

// -----------------------------------------------------------------------
// Logo displayed at the top of the log
// -----------------------------------------------------------------------
// clang-format off
const char* k_logo[] = {
    " ███  ███           ",
    "█   █  █    █    █  ",
    "█████  █  ████ ████ ",
    "█   █  █    █    █  ",
    "█   █ ███           ",
};
// clang-format on
constexpr int kLogoLines = 5;

// -----------------------------------------------------------------------
// Simple line editor state
// -----------------------------------------------------------------------
struct LineBuf {
  std::string text;
  int cursor = 0;  // byte offset into text

  void Insert(char ch) {
    text.insert(text.begin() + cursor, ch);
    ++cursor;
  }
  void Backspace() {
    if (cursor > 0) {
      text.erase(text.begin() + cursor - 1);
      --cursor;
    }
  }
  void Del() {
    if (cursor < static_cast<int>(text.size())) {
      text.erase(text.begin() + cursor);
    }
  }
  void MoveLeft() {
    if (cursor > 0) --cursor;
  }
  void MoveRight() {
    if (cursor < static_cast<int>(text.size())) ++cursor;
  }
  void Home() { cursor = 0; }
  void End() { cursor = static_cast<int>(text.size()); }

  // Return content and reset
  std::string Commit() {
    std::string result = std::move(text);
    text.clear();
    cursor = 0;
    return result;
  }
};

// -----------------------------------------------------------------------
// Log sink – keeps lines so we can scroll back
// -----------------------------------------------------------------------
struct Log {
  std::vector<std::string> lines;
  int scroll_offset = 0;

  void Append(const std::string& s) {
    lines.push_back(s);
    // auto-scroll to bottom
    scroll_offset = 0;
  }

  // How many lines fit in the log window (height h)
  static int VisibleLines(int h) { return h; }
  int TotalScrollable() const {
    int n = static_cast<int>(lines.size()) - VisibleLines(0) + 1;
    return n > 0 ? n : 0;
  }
};

// -----------------------------------------------------------------------
// Window refit helpers
// -----------------------------------------------------------------------
constexpr int kInputHeight = 3;  // prompt line + edit line + status line
constexpr int kLogTop = 0;

static void RefitLogWin(WINDOW* w, int rows, int cols) {
  wresize(w, rows - kInputHeight, cols);
  mvwin(w, kLogTop, 0);
}

static void RefitInputWin(WINDOW* w, int rows, int cols) {
  wresize(w, kInputHeight, cols);
  mvwin(w, rows - kInputHeight, 0);
}

// -----------------------------------------------------------------------
// Draw input line into the input window
// -----------------------------------------------------------------------
static void DrawInput(WINDOW* win, const LineBuf& buf, int cols) {
  werase(win);

  // prompt line
  mvwprintw(win, 0, 0, "> ");
  // If the cursor + prompt would exceed the window, scroll the display
  int prompt_cols = 2;
  int disp_offset = 0;
  int disp_len = cols - prompt_cols - 1;  // leave room
  if (disp_len < 1) disp_len = 1;
  if (buf.cursor > disp_len) disp_offset = buf.cursor - disp_len;

  int show_len = static_cast<int>(buf.text.size()) - disp_offset;
  if (show_len > disp_len) show_len = disp_len;
  if (show_len < 0) show_len = 0;

  mvwaddnstr(win, 0, prompt_cols, buf.text.c_str() + disp_offset, show_len);

  // status line
  mvwhline(win, 1, 0, ACS_HLINE, cols);
  mvwprintw(win, 2, 0, " aipp | AI++    Enter: send | Shift+Enter: newline ");

  // Place the physical cursor
  int visual_x = prompt_cols + (buf.cursor - disp_offset);
  wmove(win, 0, visual_x);
}

// -----------------------------------------------------------------------
// Redraw the entire log window
// -----------------------------------------------------------------------
static void DrawLog(WINDOW* win, const Log& log, int rows, int cols) {
  werase(win);
  int h = rows - kInputHeight;
  int start = static_cast<int>(log.lines.size()) - h - log.scroll_offset;
  if (start < 0) start = 0;
  int y = 0;
  for (int i = start; i < static_cast<int>(log.lines.size()) && y < h;
       ++i, ++y) {
    mvwaddnstr(win, y, 0, log.lines[i].c_str(), cols);
  }
}

// =======================================================================
int main() {
  if (setlocale(LC_ALL, "") == nullptr) {
    std::println(std::cerr,
                 "FATAL: setlocale() failed — check LANG/LC_ALL env vars");
    return 1;
  }

  if (initscr() == nullptr) {
    const char* term = getenv("TERM");
    if (term != nullptr) {
      std::println(std::cerr,
                   "FATAL: initscr() failed — check TERM env var (got \"{}\")",
                   term);
    } else {
      std::println(std::cerr, "FATAL: initscr() failed — TERM is not set");
    }
    return 1;
  }

  Defer endguard = {[]() noexcept {
    if (endwin() == ERR) {
      std::println(std::cerr, "endwin() failed");
    }
  }};

  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(1);  // visible cursor

  // Colour support
  if (has_colors()) {
    start_color();
    use_default_colors();
  }

  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  // Create sub-windows
  WINDOW* log_win = newwin(rows - kInputHeight, cols, kLogTop, 0);
  WINDOW* input_win = newwin(kInputHeight, cols, rows - kInputHeight, 0);
  Defer del_log_win{[log_win]() noexcept { delwin(log_win); }};
  Defer del_input_win{[input_win]() noexcept { delwin(input_win); }};
  scrollok(log_win, TRUE);
  keypad(input_win, TRUE);

  LineBuf buf;
  Log log;

  // Logo
  for (int i = 0; i < kLogoLines; ++i) log.Append(k_logo[i]);
  log.Append("");

  bool running = true;

  // Handle terminal resize
  signal(SIGWINCH, SIG_IGN);  // we'll check size manually in the loop

  while (running) {
    // --- resize check ---------------------------------------------------
    int new_rows, new_cols;
    getmaxyx(stdscr, new_rows, new_cols);
    if (new_rows != rows || new_cols != cols) {
      rows = new_rows;
      cols = new_cols;
      endwin();
      refresh();  // re-read terminfo
      RefitLogWin(log_win, rows, cols);
      RefitInputWin(input_win, rows, cols);
      clearok(stdscr, TRUE);
    }

    // --- draw -----------------------------------------------------------
    DrawLog(log_win, log, rows, cols);
    DrawInput(input_win, buf, cols);

    wnoutrefresh(log_win);
    wnoutrefresh(input_win);
    doupdate();

    // --- input ----------------------------------------------------------
    int ch = wgetch(input_win);
    switch (ch) {
      case ERR:
        break;
      case 27: {  // ESC
        // Check for escape sequences (ALT+key, arrow keys with meta, etc.)
        nodelay(input_win, TRUE);
        int next = wgetch(input_win);
        nodelay(input_win, FALSE);
        if (next == ERR) {
          running = false;  // bare ESC → quit
        } else {
          // It's an escape sequence; push back via ungetch
          ungetch(next);
        }
        break;
      }

      // Arrow keys (sent as KEY_ codes by keypad mode)
      case KEY_LEFT:
        buf.MoveLeft();
        break;
      case KEY_RIGHT:
        buf.MoveRight();
        break;
      case KEY_HOME:
        buf.Home();
        break;
      case KEY_END:
        buf.End();
        break;
      case KEY_BACKSPACE:
      case 127:
        buf.Backspace();
        break;

      // Delete (some terminals send KEY_DC, others escape sequences)
      case KEY_DC:
        buf.Del();
        break;

      // Enter
      case '\n':
      case '\r': {
        std::string line = buf.Commit();
        if (line.empty()) break;
        log.Append("> " + line);
        // TODO(aipp): send to AI
        log.Append("(AI response placeholder)");
        break;
      }

      // Printable characters
      default:
        if (ch >= 32 && ch <= 126) buf.Insert(static_cast<char>(ch));
        break;
    }
  }

  return 0;
}
