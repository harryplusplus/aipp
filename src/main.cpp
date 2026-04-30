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
  auto component = Renderer([&] {
    using namespace ftxui;
    return vbox({
        text("   ___   ___"),
        text("  / _ \\ / _ \\"),
        text(" | (_) | (_) |"),
        text("  \\___/ \\___/"),
        text("   ++   ++")
    });
});
  auto screen = ScreenInteractive::Fullscreen();
  screen.Loop(component);
  return 0;
}