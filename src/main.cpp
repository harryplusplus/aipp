#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

int main() {
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  auto component = ftxui::Renderer([] {
    return ftxui::vbox({ftxui::text("_______________"),
                        ftxui::text("___    |___  _/___________"),
                        ftxui::text("__  /| |__  / ___/ /___/ /_"),
                        ftxui::text("_  ___ |_/ /  /_  __/_  __/"),
                        ftxui::text("/_/  |_/___/   /_/   /_/")});
  });

  // Esc: 향후 다른 용도로 사용 (종료하지 않음)
  component = ftxui::CatchEvent(component, [](ftxui::Event event) {
    if (event == ftxui::Event::Escape) {
      return true;
    }
    return false;
  });

  screen.Loop(component);
  return 0;
}
