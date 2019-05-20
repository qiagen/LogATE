#include "CursATE/Screen/detail/splitIntoLines.hpp"
#include "LogATE/Utils/PrintableStringConverter.hpp"
#include <But/assert.hpp>
#include <sstream>
#include <iostream>                 
#include <algorithm>


namespace CursATE::Screen::detail
{

namespace
{
auto isEscape(const char c) { return c == '\\'; }
auto isCr(const char c) { return c == 'n'; }
auto isLf(const char c) { return c == 'r'; }
auto isEolChar(const char c) { return isCr(c) || isLf(c); }

auto hasEscaped(char const c, const std::string::const_iterator begin, const std::string::const_iterator end)
{
  if(begin==end)
    return false;
  if( not isEscape(*begin) )
    return false;
  const auto it = begin + 1;
  BUT_ASSERT(it!=end && "invalid sequence provided - break should not be in the middle of an escape sequence");
  return *it==c;
}
auto hasEscapedCr(const std::string::const_iterator begin, const std::string::const_iterator end)  { return hasEscaped('n', begin, end); }
auto hasEscapedLf(const std::string::const_iterator begin, const std::string::const_iterator end)  { return hasEscaped('r', begin, end); }
auto hasEscapedTab(const std::string::const_iterator begin, const std::string::const_iterator end) { return hasEscaped('t', begin, end); }

auto findEndOfEscapeSequence(const std::string::const_iterator begin, const std::string::const_iterator end)
{
  BUT_ASSERT(begin!=end);
  BUT_ASSERT( isEscape(*begin) );
  const auto next = begin+1;
  if(*next=='x')
  {
    BUT_ASSERT( std::distance(begin, end) >= 2 );
    return next+2;
  }
  return next;
}

auto findFirstEol(const std::string::const_iterator begin, const std::string::const_iterator end)
{
  for(auto it=begin; it!=end; ++it)
  {
    if( hasEscapedCr(it, end) )
    {
      if( hasEscapedLf(it+2, end) )
        return it+4;
      return it+2;
    }
    if( hasEscapedLf(it, end) )
    {
      if( hasEscapedCr(it+2, end) )
        return it+4;
      return it+2;
    }
    if( isEscape(*it) )
    {
      it = findEndOfEscapeSequence(it, end);
      BUT_ASSERT(it!=end && "invalid sequence - cannot end with broken escape sequence");
    }
  }
  return end;
}

auto findLastInLineBeforeSplitEscape(const std::string::const_iterator begin, const std::string::const_iterator end, const size_t cols)
{
  const auto inRange = static_cast<size_t>( std::distance(begin, end) );
  const auto effectiveEnd = ( inRange >= cols ) ? begin+cols : end;
  for(auto it=begin; it!=effectiveEnd; ++it)
  {
    if( not isEscape(*it) )
      continue;
    it = findEndOfEscapeSequence(it, effectiveEnd);
    if(it+1 == effectiveEnd)
      return it;
    ++it;
  }
  return effectiveEnd;
}


auto findLastWhitespace(const std::string::const_iterator begin, const std::string::const_iterator end)
{
  auto lastWs = end;
  for(auto it=begin; it!=end; ++it)
  {
    if(*it == ' ')
    {
      lastWs = it+1;
      continue;
    }
    if( hasEscapedTab(it, end) )
    {
      lastWs = it+2;
      continue;
    }
  }
  return lastWs;
}

auto isWhiteSpace(const std::string::const_iterator begin, const std::string::const_iterator end)
{
  BUT_ASSERT(begin!=end);
  if(*begin == ' ')
    return true;
  if( hasEscapedTab(begin, end) )
    return true;
  return false;
}

auto findNextLineBreak(const std::string::const_iterator begin, const std::string::const_iterator end, const size_t cols)
{
  const auto newEnd = findLastInLineBeforeSplitEscape(begin, end, cols);
  std::cout << "\nEE:|" << std::string{begin, newEnd} << "| of |" << std::string{begin, end} << "| @ " << cols << " cols" << std::endl;         
  const auto firstEol = findFirstEol(begin, newEnd);
  std::cout << "EL:|" << std::string{begin, firstEol} << "|" << std::endl;                   
  if(firstEol != newEnd)
    return firstEol;
  if(firstEol == end)
    return firstEol;
  if( isWhiteSpace(firstEol, end) )
    return firstEol;
  const auto lastWs = findLastWhitespace(begin, firstEol);
  std::cout << "WS:|" << std::string{begin, lastWs} << "|" << std::endl;                   
  return lastWs;
}
}

std::vector<std::string> splitIntoLines(std::string const& in, const size_t availableColumns)
{
  if( in.empty() )
    return {};
  if( not std::all_of( begin(in), end(in), isprint ) )
    std::logic_error{"detail::splitIntoLines(): input sequence shall already be escaped"};
  const auto columns = std::max<size_t>(1u, availableColumns);
  std::vector<std::string> out;
  auto it = begin(in);
  while( it != end(in) )
  {
    const auto lb = findNextLineBreak(it, end(in), columns);
    out.emplace_back(it, lb);
    it = lb;
  }
  return out;
}

}