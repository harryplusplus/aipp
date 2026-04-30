#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

int main() {
    using namespace ftxui;
    const std::string ascii_art = R"(
   ___   ___
  / _ \ / _ \
 | (_) | (_) |
  \___/ \___/
   ++   ++
)";
    auto component = Renderer([&] { return text(ascii_art); });
    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(component);
    return 0;
}