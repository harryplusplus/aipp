#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

int run_tui() {
    const std::string ascii_art = R"(
   ___   ___
  / _ \\ / _ \\
 | (_) | (_) |
  \\___/ \\___/
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

int run_cli() {
    std::cout << "AI++ CLI mode: ready to accept commands." << std::endl;
    // TODO: implement actual AI agent logic here.
    return 0;
}

int main(int argc, char** argv) {
    CLI::App app("AI++ Agent Harness");
    bool tui = false;
    app.add_flag("-t,--tui", tui, "Run in TUI mode (default is CLI mode)");
    CLI11_PARSE(app, argc, argv);
    if (tui) {
        return run_tui();
    } else {
        return run_cli();
    }
}
