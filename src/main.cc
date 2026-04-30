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

  // 입력 영역
  ftxui::InputOption input_opt;
  input_opt.multiline = true;
  input_opt.max_visible_lines = 11;
  input_opt.transform = [](ftxui::InputState state) {
    ftxui::Elements elems;

    int above = state.scroll_offset;
    int below = state.total_lines - state.scroll_offset - state.visible_lines;

    // 위쪽 separator + indicator (dbox로 겹쳐서 수직선 없음)
    if (above > 0) {
      elems.push_back(ftxui::dbox({
          ftxui::separator(),
          ftxui::text("\u2500\u2500\u2500 \u2191 " + std::to_string(above) + " more \u2500\u2500\u2500"),
      }));
    } else {
      elems.push_back(ftxui::separator());
    }

    elems.push_back(state.element);

    // 아래쪽 separator + indicator (dbox로 겹쳐서 수직선 없음)
    if (below > 0) {
      elems.push_back(ftxui::dbox({
          ftxui::separator(),
          ftxui::text("\u2500\u2500\u2500 \u2193 " + std::to_string(below) + " more \u2500\u2500\u2500"),
      }));
    } else {
      elems.push_back(ftxui::separator());
    }

    return ftxui::vbox(std::move(elems));
  };
  auto input = ftxui::Input(&input_content, input_opt);

  // Footer
  auto footer = ftxui::Renderer([] {
    return ftxui::text(" aipp | AI++    Enter: send | Shift+Enter: newline ");
  });

  // 전체 뷰 (Container 없이 직접 조합)
  // Input의 transform이 위/아래 separator를 포함하므로 별도 separator 불필요.
  auto view = ftxui::Renderer([&] {
    return ftxui::vbox({
        output->Render(),
        input->Render(),
        footer->Render(),
    });
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
    return input->OnEvent(event);
  });

  screen.TrackMouse(false);
  screen.Loop(view);
  return 0;
}
