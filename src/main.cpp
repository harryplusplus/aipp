#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

int main() {
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  std::string input_content;

  // 출력 영역 (로고 + 추후 AI 응답)
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
  auto input = ftxui::Input(&input_content, input_opt) | ftxui::border;

  auto container = ftxui::Container::Vertical({
      output,
      input,
  });
  container->SetActiveChild(input);

  // 전역 키 처리
  container = ftxui::CatchEvent(container, [&](ftxui::Event event) {
    if (event == ftxui::Event::Escape) {
      return true;  // Esc: 향후 다른 용도로 사용 (종료하지 않음)
    }
    if (event == ftxui::Event::Return) {
      // TODO: input_content 전송 → AI
      return true;  // Enter → 새 줄 없이 submit
    }
    // Shift+Enter → 새 줄 (터미널 CSI u: \x1B[27;2;13~)
    if (event.input() == "\x1B[27;2;13~") {
      input_content += '\n';
      screen.PostEvent(ftxui::Event::End);  // 커서를 끝으로
      return true;
    }
    return false;  // 그 외는 Input이 처리
  });

  screen.Loop(container);
  return 0;
}
