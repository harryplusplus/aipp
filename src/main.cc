#include <ncurses.h>

#include <string>
#include <vector>

// -----------------------------------------------------------------------
// Simple line editor state
// -----------------------------------------------------------------------
struct LineBuf {
  std::string text;
  int cursor = 0;  // byte offset into text

  void insert(char ch) {
    text.insert(text.begin() + cursor, ch);
    ++cursor;
  }
  void backspace() {
    if (cursor > 0) {
      text.erase(text.begin() + cursor - 1);
      --cursor;
    }
  }
  void del() {
    if (cursor < static_cast<int>(text.size()))
      text.erase(text.begin() + cursor);
  }
  void move_left() {
    if (cursor > 0) --cursor;
  }
  void move_right() {
    if (cursor < static_cast<int>(text.size())) ++cursor;
  }
  void home() { cursor = 0; }
  void end() { cursor = text.size(); }

  // Return content and reset
  std::string commit() {
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

  void append(const std::string& s) {
    lines.push_back(s);
    // auto-scroll to bottom
    scroll_offset = 0;
  }

  // How many lines fit in the log window (height h)
  int visible_lines(int h) const { return h; }
  int total_scrollable() const {
    int n = static_cast<int>(lines.size()) - visible_lines(0) + 1;
    return n > 0 ? n : 0;
  }
};

// -----------------------------------------------------------------------
// Window refit helpers
// -----------------------------------------------------------------------
constexpr int INPUT_HEIGHT = 3;  // prompt line + edit line + status line
constexpr int LOG_TOP = 0;

static void refit_log_win(WINDOW* w, int rows, int cols) {
  wresize(w, rows - INPUT_HEIGHT, cols);
  mvwin(w, LOG_TOP, 0);
}

static void refit_input_win(WINDOW* w, int rows, int cols) {
  wresize(w, INPUT_HEIGHT, cols);
  mvwin(w, rows - INPUT_HEIGHT, 0);
}

// -----------------------------------------------------------------------
// Draw input line into the input window
// -----------------------------------------------------------------------
static void draw_input(WINDOW* win, const LineBuf& buf, int cols) {
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
static void draw_log(WINDOW* win, const Log& log, int rows, int cols) {
  werase(win);
  int h = rows - INPUT_HEIGHT;
  int start = static_cast<int>(log.lines.size()) - h - log.scroll_offset;
  if (start < 0) start = 0;
  int y = 0;
  for (int i = start; i < static_cast<int>(log.lines.size()) && y < h; ++i, ++y)
    mvwaddnstr(win, y, 0, log.lines[i].c_str(), cols);
}

// =======================================================================
int main() {
  initscr();
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
  WINDOW* log_win = newwin(rows - INPUT_HEIGHT, cols, LOG_TOP, 0);
  WINDOW* input_win = newwin(INPUT_HEIGHT, cols, rows - INPUT_HEIGHT, 0);
  scrollok(log_win, TRUE);
  keypad(input_win, TRUE);

  LineBuf buf;
  Log log;

  // Sample placeholder
  log.append("  ___    |___  _/___________");
  log.append("  __  /| |__  / ___/ /___/ /_");
  log.append("  _  ___ |_/ /  /_  __/_  __/");
  log.append("  /_/  |_/___/   /_/   /_/");
  log.append("");

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
      refit_log_win(log_win, rows, cols);
      refit_input_win(input_win, rows, cols);
      clearok(stdscr, TRUE);
    }

    // --- draw -----------------------------------------------------------
    draw_log(log_win, log, rows, cols);
    draw_input(input_win, buf, cols);

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
        buf.move_left();
        break;
      case KEY_RIGHT:
        buf.move_right();
        break;
      case KEY_HOME:
        buf.home();
        break;
      case KEY_END:
        buf.end();
        break;
      case KEY_BACKSPACE:
      case 127:
        buf.backspace();
        break;

      // Delete (some terminals send KEY_DC, others escape sequences)
      case KEY_DC:
        buf.del();
        break;

      // Enter
      case '\n':
      case '\r': {
        std::string line = buf.commit();
        if (line.empty()) break;
        log.append("> " + line);
        // TODO: send to AI
        log.append("(AI response placeholder)");
        break;
      }

      // Printable characters
      default:
        if (ch >= 32 && ch <= 126) buf.insert(static_cast<char>(ch));
        break;
    }
  }

  delwin(log_win);
  delwin(input_win);
  endwin();
  return 0;
}
