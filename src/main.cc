#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

int main() {
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  std::string input_content;
  int cursor_line = 0;

  // 출력 영역 (로고 + AI 응답)
  auto output = ftxui::Renderer([] {
    return ftxui::vbox({
               ftxui::text("_______________"),
               ftxui::text("___    |___  _/___________"),
               ftxui::text("__  /| |__  / ___/ /___/ /_"),
               ftxui::text("_  ___ |_/ /  /_  __/_  __/"),
               ftxui::text("/_/  |_/___/   /_/   /_/"),
               ftxui::separator(),
               // TODO: AI response area
           }) |
           ftxui::flex;
  });

  // 입력 영역
  ftxui::InputOption input_opt;
  input_opt.multiline = true;
  input_opt.on_change = [&] {
    int lines = 1;
    for (char c : input_content)
      if (c == '\n') lines++;
    cursor_line = lines - 1;
  };
  auto input = ftxui::Input(&input_content, input_opt) |
               ftxui::size(ftxui::HEIGHT, ftxui::GREATER_THAN, 1) |
               ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 11);

  // Footer
  auto footer = ftxui::Renderer([] {
    return ftxui::hbox({
        ftxui::text(" aipp "),
        ftxui::separator(),
        ftxui::text(" AI++ "),
        ftxui::filler(),
        ftxui::text(" Enter: send | Shift+Enter: newline "),
    });
  });

  // 전체 뷰 (Container 없이 직접 조합)
  auto view = ftxui::Renderer([&] {
    int total = 1;
    for (char c : input_content)
      if (c == '\n') total++;

    ftxui::Elements elems;
    elems.push_back(output->Render());

    elems.push_back(ftxui::separator());

    // 스크롤 인디케이터 (위)
    if (cursor_line > 0) {
      elems.push_back(
          ftxui::text("  \u2191 " + std::to_string(cursor_line) + " more "));
    }

    elems.push_back(input->Render());

    // 스크롤 인디케이터 (아래)
    int after = total - cursor_line - 1;
    if (after > 0) {
      elems.push_back(
          ftxui::text("  \u2193 " + std::to_string(after) + " more "));
    }

    elems.push_back(ftxui::separator());
    elems.push_back(footer->Render());

    return ftxui::vbox(elems);
  });

  // 키 이벤트
  view = ftxui::CatchEvent(view, [&](ftxui::Event event) {
    if (event == ftxui::Event::Escape) {
      return true;
    }
    if (event == ftxui::Event::Return) {
      // TODO: input_content 전송 → AI
      return true;
    }
    if (event.input() == "\x1B[27;2;13~") {
      input_content += '\n';
      screen.PostEvent(ftxui::Event::End);
      return true;
    }
    if (event == ftxui::Event::ArrowUp) {
      if (cursor_line > 0) cursor_line--;
      return input->OnEvent(event);
    }
    if (event == ftxui::Event::ArrowDown) {
      int total = 1;
      for (char c : input_content)
        if (c == '\n') total++;
      if (cursor_line + 1 < total) cursor_line++;
      return input->OnEvent(event);
    }
    return input->OnEvent(event);
  });

  screen.Loop(view);
  return 0;
}
