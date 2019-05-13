#include "CursATE/Screen/detail/FilterWindows.hpp"
#include "CursATE/Screen/detail/LogDataSource.hpp"
#include "CursATE/Screen/detail/formatAsPercentage.hpp"

namespace CursATE::Screen::detail
{

FilterWindows::FilterWindows(std::function<std::string(LogATE::Log const&)> log2str,
                             std::function<size_t()> inputErrors):
  log2str_{ std::move(log2str) },
  inputErrors_{ std::move(inputErrors) }
{ }


But::NotNullShared<Curses::ScrolableWindow> FilterWindows::window(LogATE::Tree::NodeShPtr const& node)
{
  auto f = [&](auto const& e) { return e.node_.lock().get() == node.get(); };
  const auto it = std::find_if( begin(entries_), end(entries_), f );
  if( it != end(entries_) )
    return it->win_;
  const auto win = newWindow(node);
  win->selectLast();
  entries_.push_back( Element{ node.underlyingPointer(), win } );
  return win;
}


void FilterWindows::prune()
{
  auto f = [&](auto const& e) { return e.node_.lock().get() == nullptr; };
  const auto it = std::remove_if( begin(entries_), end(entries_), f );
  entries_.erase(it, end(entries_));
}


But::NotNullShared<Curses::ScrolableWindow> FilterWindows::newWindow(LogATE::Tree::NodeShPtr node) const
{
  const auto ds = But::makeSharedNN<LogDataSource>( std::move(node), log2str_ );
  const auto sp = Curses::ScreenPosition{Curses::Row{0}, Curses::Column{0}};
  const auto ss = Curses::ScreenSize::global();
  const auto boxed = Curses::Window::Boxed::True;
  auto status = [ds, errCnt = inputErrors_](const size_t pos) { return detail::nOFmWithPercent(pos, ds->size()) +
                                                              " E:" + std::to_string( errCnt() ); };
  return But::makeSharedNN<Curses::ScrolableWindow>(ds, sp, ss, boxed, std::move(status));
}

}
