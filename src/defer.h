// -----------------------------------------------------------------------
// Defer — Go-style deferred cleanup.
//
//   initscr();
//   Defer endguard = { endwin };
//
//   WINDOW* w = newwin(...);
//   Defer del_w = { [&] { delwin(w); } };
//
// Cleanup is called when the Defer goes out of scope (reverse order of
// construction).  Accepts any callable (function pointer, lambda with
// captures, etc.) via CTAD.
// -----------------------------------------------------------------------
#pragma once

#include <concepts>
#include <type_traits>

template <typename F>
  requires std::invocable<F> && std::same_as<void, std::invoke_result_t<F>> &&
           std::is_nothrow_invocable_v<F>
class Defer {
 public:
  Defer(F fn) : fn_(fn) {}
  ~Defer() noexcept { fn_(); }

  Defer(const Defer&) = delete;
  Defer(Defer&&) = delete;
  Defer& operator=(const Defer&) = delete;
  Defer& operator=(Defer&&) = delete;

 private:
  F fn_;
};
