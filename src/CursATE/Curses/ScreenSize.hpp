#pragma once
#include <ncurses.h>
#include <But/Mpl/FreeOperators.hpp>
#include <But/assert.hpp>

namespace CursATE::Curses
{

struct Columns { int value_{0}; };
BUT_MPL_FREE_OPERATORS_COMPARE(Columns, .value_)

struct Rows { int value_{0}; };
BUT_MPL_FREE_OPERATORS_COMPARE(Rows, .value_)

struct ScreenSize
{
  static auto global() { return ScreenSize{stdscr}; }

  explicit ScreenSize(WINDOW* screen)
  {
    BUT_ASSERT(screen);
    getmaxyx(screen, rows_.value_, columns_.value_);
  }
  ScreenSize(const Rows rows, const Columns columns):
    rows_{rows},
    columns_{columns}
  { }

  Rows rows_;
  Columns columns_;
};


inline auto operator==(const ScreenSize lhs, const ScreenSize rhs)
{
  return lhs.rows_ == rhs.rows_ && lhs.columns_ == rhs.columns_;
}

}
