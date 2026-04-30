#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

int main() {

  const std::string ascii_art = R"(
   ___   ___
  / _ \ / _ \
 | (_) | (_) |
  \___/ \___/
   ++   ++
)";
  auto component = ftxui::Renderer([&] {
    
    return ftxui::vbox({
        ftxui::text("   ___   ___"),
        ftxui::text("  / _ \\ / _ \\"),
        ftxui::text(" | (_) | (_) |"),
        ftxui::text("  \\___/ \\___/"),
        ftxui::text("   ++   ++")
    });
});
  auto screen = ftxui::ScreenInteractive::Fullscreen();
  screen.Loop(component);
  return 0;
}