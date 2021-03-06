#include <doctest/doctest.h>
#include "LogATE/Tree/detail/LogKeyIndexCache.hpp"
#include "LogATE/TestHelpers.ut.hpp"
#include <sstream>
#include <random>

using LogATE::makeKey;

namespace
{
TEST_SUITE("LogATE::Tree::detail::LogKeyIndexCache")
{

struct Fixture
{
  static auto makeLog(unsigned sn, std::string key) { return LogATE::Log{ makeKey(sn, std::move(key)), "{}" }; }

  void fillWithData()
  {
    insert( makeLog(1, "bar") );
    insert( makeLog(4, "bar") );
    insert( makeLog(0, "foo") );
    insert( makeLog(8, "kamboom!") );
    insert( makeLog(5, "kszy") );
    insert( makeLog(3, "narf") );
  }

  void insert(LogATE::Log const& log)
  {
    data_.insert(log);
    liic_.updateAfterInsertion( log.key() );
  }

  void pruneUpTo(LogATE::Log::Key const& key)
  {
    while( not data_.empty() && data_.begin()->key() < key )
      data_.erase( data_.begin() );
    liic_.updateAfterPruneUpTo(key);
  }

  auto index(LogATE::Log::Key const& key) { return liic_.index(key); }

  LogATE::Tree::detail::LogKeyIndexCache::Data data_;
  LogATE::Tree::detail::LogKeyIndexCache liic_{&data_, 100'000, 0};
};


TEST_CASE_FIXTURE(Fixture, "empty node returns 0")
{
  CHECK( index( makeKey(666) ) == 0u );
  CHECK( liic_.size() == 0u );
}


TEST_CASE_FIXTURE(Fixture, "non-empty node but w/o given element returns 0")
{
  fillWithData();

  SUBCASE("element is off on the right")
  {
    CHECK( index( makeKey(666) ) == 0u );
    CHECK( liic_.size() == 0u );
  }
  SUBCASE("element is off on the left")
  {
    CHECK( index( makeKey(0, "aaa") ) == 0u );
    CHECK( liic_.size() == 0u );
  }
  SUBCASE("element is off to the left, but was cached")
  {
    CHECK( index( makeKey(1, "bar") ) == 0 );
    CHECK( liic_.size() == 1u );
    CHECK( index( makeKey(4, "bar") ) == 1 );
    CHECK( liic_.size() == 2u );
    pruneUpTo( makeKey(4, "bar") );
    CHECK( liic_.size() == 1u );
    CHECK( index( makeKey(1, "bar") ) == 0 );
    CHECK( liic_.size() == 1u );
    CHECK( index( makeKey(4, "bar") ) == 0 );
    CHECK( liic_.size() == 1u );
  }
}


TEST_CASE_FIXTURE(Fixture, "finding elements in node")
{
  fillWithData();
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(1, "bar") ) == 0 );
  CHECK( liic_.size() == 1u );
  CHECK( index( makeKey(4, "bar") ) == 1 );
  CHECK( liic_.size() == 2u );
  CHECK( index( makeKey(0, "foo") ) == 2 );
  CHECK( liic_.size() == 3u );
  CHECK( index( makeKey(8, "kamboom!") ) == 3 );
  CHECK( liic_.size() == 4u );
  CHECK( index( makeKey(5, "kszy") ) == 4 );
  CHECK( liic_.size() == 5u );
  CHECK( index( makeKey(3, "narf") ) == 5 );
  CHECK( liic_.size() == 6u );
  // repeated search
  CHECK( index( makeKey(8, "kamboom!") ) == 3 );
  CHECK( liic_.size() == 6u );

  SUBCASE("cache gets ipdated after insertion in the middle of the data set")
  {
    insert( makeLog(666, "gemini") );
    // repeated searches
    CHECK( liic_.size() == 6u );
    CHECK( index( makeKey(1, "bar") ) == 0 );
    CHECK( liic_.size() == 6u );
    CHECK( index( makeKey(4, "bar") ) == 1 );
    CHECK( liic_.size() == 6u );
    CHECK( index( makeKey(0, "foo") ) == 2 );
    CHECK( liic_.size() == 6u );
    CHECK( index( makeKey(8, "kamboom!") ) == 4 );
    CHECK( liic_.size() == 6u );
    CHECK( index( makeKey(5, "kszy") ) == 5 );
    CHECK( liic_.size() == 6u );
    CHECK( index( makeKey(3, "narf") ) == 6 );
    CHECK( liic_.size() == 6u );
    // search for new element
    CHECK( index( makeKey(666, "gemini") ) == 3 );
    CHECK( liic_.size() == 7u );
  }
}


TEST_CASE_FIXTURE(Fixture, "auto-cleanup of outdated entries")
{
  fillWithData();
  pruneUpTo( makeKey(0, "foo") );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(1, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(4, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(0, "foo") ) == 0 );
  CHECK( liic_.size() == 1u );
  CHECK( index( makeKey(8, "kamboom!") ) == 1 );
  CHECK( liic_.size() == 2u );
  CHECK( index( makeKey(5, "kszy") ) == 2 );
  CHECK( liic_.size() == 3u );
  CHECK( index( makeKey(3, "narf") ) == 3 );
  CHECK( liic_.size() == 4u );

  pruneUpTo( makeKey(666, "xxx") );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(1, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(4, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(0, "foo") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(8, "kamboom!") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(5, "kszy") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(3, "narf") ) == 0 );
  CHECK( liic_.size() == 0u );
}


TEST_CASE_FIXTURE(Fixture, "test working on 1-element set")
{
  insert( makeLog(0, "foo") );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(1, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(4, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( index( makeKey(0, "foo") ) == 0 );
  CHECK( liic_.size() == 1u );
  CHECK( index( makeKey(8, "kamboom!") ) == 0 );
  CHECK( liic_.size() == 1u );
  CHECK( index( makeKey(5, "kszy") ) == 0 );
  CHECK( liic_.size() == 1u );
  CHECK( index( makeKey(3, "narf") ) == 0 );
  CHECK( liic_.size() == 1u );
}


TEST_CASE_FIXTURE(Fixture, "test random scenarios")
{
  fillWithData();
  CHECK( liic_.size() == 0u );

  SUBCASE("test first elements")
  {
    CHECK( index( makeKey(1, "bar") ) == 0 );
    CHECK( liic_.size() == 1u );
  }
  SUBCASE("last element")
  {
    CHECK( index( makeKey(3, "narf") ) == 5 );
    CHECK( liic_.size() == 1u );
  }
  SUBCASE("middle element")
  {
    CHECK( index( makeKey(8, "kamboom!") ) == 3 );
    CHECK( liic_.size() == 1u );
  }
  SUBCASE("element in between different ones")
  {
    CHECK( index( makeKey(4, "bar") ) == 1 );
    CHECK( liic_.size() == 1u );
    CHECK( index( makeKey(5, "kszy") ) == 4 );
    CHECK( liic_.size() == 2u );
    CHECK( index( makeKey(8, "kamboom!") ) == 3 );
    CHECK( liic_.size() == 3u );
  }
  SUBCASE("search elements in backward order")
  {
    CHECK( index( makeKey(3, "narf") ) == 5 );
    CHECK( liic_.size() == 1u );
    CHECK( index( makeKey(5, "kszy") ) == 4 );
    CHECK( liic_.size() == 2u );
    CHECK( index( makeKey(8, "kamboom!") ) == 3 );
    CHECK( liic_.size() == 3u );
  }
}


auto randomData()
{
  std::vector<LogATE::Log> out;
  const auto alphabet = {0,1,2,3,4,5,6,7,8,9};
  auto counter = 0u;
  for(auto i: {0,1,2})
    for(auto j: alphabet)
      for(auto k: alphabet)
          out.push_back( Fixture::makeLog( counter++, std::to_string(i) + std::to_string(j) + std::to_string(k) ) );
  return out;
}

auto randomQueries(std::vector<LogATE::Log> const& in, unsigned seed)
{
  std::vector<std::pair<LogATE::Log::Key, size_t>> out;
  out.reserve( in.size() );
  auto pos = 0u;
  for(auto& e: in)
    out.push_back( std::make_pair( e.key(), pos++ ) );
  std::mt19937_64 prng{seed};
  std::shuffle( begin(out), end(out), prng );
  return out;
}

TEST_CASE_FIXTURE(Fixture, "random scenarios - big data sets")
{
  const auto data = randomData();
  for(auto& e: data)
    insert(e);
  CHECK( liic_.size() == 0u );

  SUBCASE("order 1")
  {
    for(auto& q: randomQueries(data, 40+1))
      CHECK( index(q.first) == q.second );
  }
  SUBCASE("order 2")
  {
    for(auto& q: randomQueries(data, 40+2))
      CHECK( index(q.first) == q.second );
  }
}


TEST_CASE_FIXTURE(Fixture, "auto-caching every Nth added element")
{
  LogATE::Tree::detail::LogKeyIndexCache liic{&data_, 3, 0};
  CHECK( liic.size() == 0u );
  auto ins = [&](auto const& log) {
    data_.insert(log);
    liic.updateAfterInsertion( log.key() );
  };

  ins( makeLog(42, "aaa") );
  CHECK( liic.size() == 0u );

  ins( makeLog(43, "bbb") );
  CHECK( liic.size() == 0u );

  ins( makeLog(44, "ccc") );
  CHECK( liic.size() == 1u );

  ins( makeLog(45, "ddd") );
  CHECK( liic.size() == 1u );

  ins( makeLog(46, "eee") );
  CHECK( liic.size() == 1u );

  ins( makeLog(47, "fff") );
  CHECK( liic.size() == 2u );
}


TEST_CASE_FIXTURE(Fixture, "prevent caching entries that are close to already existing ones")
{
  LogATE::Tree::detail::LogKeyIndexCache liic{&data_, 1000, 2};

  data_.insert( makeLog(42, "aa") );  // 0
  data_.insert( makeLog(43, "bb") );  // 1
  data_.insert( makeLog(44, "cc") );  // 2
  data_.insert( makeLog(45, "dd") );  // 3
  data_.insert( makeLog(46, "ee") );  // 4 <-- you are here...  #1
  data_.insert( makeLog(47, "ff") );  // 5
  data_.insert( makeLog(48, "gg") );  // 6
  data_.insert( makeLog(49, "hh") );  // 7
  data_.insert( makeLog(50, "ii") );  // 8
  data_.insert( makeLog(51, "jj") );  // 9
  data_.insert( makeLog(52, "kk") );  // 10 <-- ...and here     #2
  data_.insert( makeLog(53, "ll") );  // 11
  data_.insert( makeLog(54, "mm") );  // 12
  data_.insert( makeLog(55, "nn") );  // 13
  data_.insert( makeLog(56, "oo") );  // 14

  REQUIRE( liic.size() == 0u );

  CHECK( liic.index( makeKey(46, "ee") ) == 4 );
  CHECK( liic.size() == 1u );
  CHECK( liic.index( makeKey(52, "kk") ) == 10 );
  CHECK( liic.size() == 2u );

  SUBCASE("at the begining")
  {
    CHECK( liic.index( makeKey(42, "aa") ) == 0 );
    CHECK( liic.size() == 3u );
  }
  SUBCASE("far before #1")
  {
    CHECK( liic.index( makeKey(43, "bb") ) == 1 );
    CHECK( liic.size() == 3u );
  }
  SUBCASE("close before #1")
  {
    CHECK( liic.index( makeKey(44, "cc") ) == 2 );
    CHECK( liic.size() == 2u );
    CHECK( liic.index( makeKey(45, "dd") ) == 3 );
    CHECK( liic.size() == 2u );
  }
  SUBCASE("close after #1")
  {
    CHECK( liic.index( makeKey(47, "ff") ) == 5 );
    CHECK( liic.size() == 2u );
    CHECK( liic.index( makeKey(48, "gg") ) == 6 );
    CHECK( liic.size() == 2u );
  }

  SUBCASE("in between #1 and #2")
  {
    CHECK( liic.index( makeKey(49, "hh") ) == 7 );
    CHECK( liic.size() == 3u );
  }

  SUBCASE("close before #2")
  {
    CHECK( liic.index( makeKey(50, "ii") ) == 8 );
    CHECK( liic.size() == 2u );
    CHECK( liic.index( makeKey(51, "jj") ) == 9 );
    CHECK( liic.size() == 2u );
  }
  SUBCASE("close after #2")
  {
    CHECK( liic.index( makeKey(53, "ll") ) == 11 );
    CHECK( liic.size() == 2u );
    CHECK( liic.index( makeKey(54, "mm") ) == 12 );
    CHECK( liic.size() == 2u );
  }
  SUBCASE("far after #2")
  {
    CHECK( liic.index( makeKey(55, "nn") ) == 13 );
    CHECK( liic.size() == 3u );
  }
  SUBCASE("at the end")
  {
    CHECK( liic.index( makeKey(56, "oo") ) == 14 );
    CHECK( liic.size() == 3u );
  }
}


TEST_CASE_FIXTURE(Fixture, "prevent caching entries that are close to already existing ones at the edge of the set")
{
  LogATE::Tree::detail::LogKeyIndexCache liic{&data_, 1000, 2};

  data_.insert( makeLog(42, "aa") );  // 0
  data_.insert( makeLog(43, "bb") );  // 1
  data_.insert( makeLog(44, "cc") );  // 2
  data_.insert( makeLog(45, "dd") );  // 3
  data_.insert( makeLog(46, "ee") );  // 4

  REQUIRE( liic.size() == 0u );

  SUBCASE("begining cached")
  {
    CHECK( liic.index( makeKey(42, "aa") ) == 0 );
    CHECK( liic.size() == 1u );
    CHECK( liic.index( makeKey(43, "bb") ) == 1 );
    CHECK( liic.size() == 1u );
  }
  SUBCASE("next after begining cached")
  {
    CHECK( liic.index( makeKey(43, "bb") ) == 1 );
    CHECK( liic.size() == 1u );
    CHECK( liic.index( makeKey(42, "aa") ) == 0 );
    CHECK( liic.size() == 1u );
  }
  SUBCASE("end cached")
  {
    CHECK( liic.index( makeKey(46, "ee") ) == 4 );
    CHECK( liic.size() == 1u );
    CHECK( liic.index( makeKey(45, "dd") ) == 3 );
    CHECK( liic.size() == 1u );
  }
  SUBCASE("one before end cached")
  {
    CHECK( liic.index( makeKey(45, "dd") ) == 3 );
    CHECK( liic.size() == 1u );
    CHECK( liic.index( makeKey(46, "ee") ) == 4 );
    CHECK( liic.size() == 1u );
  }
}


TEST_CASE_FIXTURE(Fixture, "caching prevention mechanism is disabled when distance set to zero")
{
  LogATE::Tree::detail::LogKeyIndexCache liic{&data_, 1000, 0};

  data_.insert( makeLog(42, "aa") );  // 0
  data_.insert( makeLog(43, "bb") );  // 1
  data_.insert( makeLog(44, "cc") );  // 2

  REQUIRE( liic.size() == 0u );

  CHECK( liic.index( makeKey(42, "aa") ) == 0 );
  CHECK( liic.size() == 1u );

  CHECK( liic.index( makeKey(43, "bb") ) == 1 );
  CHECK( liic.size() == 2u );

  CHECK( liic.index( makeKey(44, "cc") ) == 2 );
  CHECK( liic.size() == 3u );
}

}
}
