#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

int main() {
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  std::string input_content;

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

  // 입력 영역 (border 없음, 최대 11줄)
  ftxui::InputOption input_opt;
  input_opt.multiline = true;
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

  // Container::Vertical 없이 직접 조합
  auto view = ftxui::Renderer([&] {
    return ftxui::vbox({
        output->Render(),
        ftxui::separator(),
        input->Render(),
        ftxui::separator(),
        footer->Render(),
    });
  });

  // 키 이벤트 처리
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
    // 그 외 모든 키 (문자, Arrow, Backspace, Delete, Home, End 등)는 Input이
    // 처리
    return input->OnEvent(event);
  });

  screen.Loop(view);
  return 0;
}
