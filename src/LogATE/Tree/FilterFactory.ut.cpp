#include <doctest/doctest.h>
#include "LogATE/Tree/FilterFactory.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Tree::FilterFactory;
using LogATE::Tree::Node;
using LogATE::Tree::Path;
using Opts = LogATE::Tree::FilterFactory::Options;

namespace
{
TEST_SUITE("Tree::FilterFactory")
{

struct Fixture
{
  const Node::Name name_{"foo-bar"};
  FilterFactory ff_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
};


TEST_CASE_FIXTURE(Fixture, "constructing AcceptAll filter")
{
  const auto type = FilterFactory::Type{"AcceptAll"};

  SUBCASE("valid")
  {
    const auto ptr = ff_.build( type, name_, Opts{} );
    CHECK( ptr->type().value_ == type.name_ );
    CHECK( ptr->trimFields().empty() );
  }

  SUBCASE("optional field trimming")
  {
    const auto ptr = ff_.build( type, name_, Opts{{"Trim", "[\".foo.bar\"]"}} );
    CHECK( ptr->type().value_ == type.name_ );
    const auto tf = ptr->trimFields();
    REQUIRE( tf.size() == 1 );
    CHECK( tf[0] == Path::parse(".foo.bar") );
  }

  SUBCASE("field trimming works for multiple elements")
  {
    const auto ptr = ff_.build( type, name_, Opts{{"Trim", "[\".foo.bar\", \".some.other.value\"]"}} );
    CHECK( ptr->type().value_ == type.name_ );
    const auto tf = ptr->trimFields();
    REQUIRE( tf.size() == 2 );
    CHECK( tf[0] == Path::parse(".foo.bar") );
    CHECK( tf[1] == Path::parse(".some.other.value") );
  }

  SUBCASE("unknown argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{{"foo", "bar"}} ), FilterFactory::UnknownOption );
  }

  SUBCASE("invalid argument")
  {
    CHECK_THROWS( ff_.build( type, name_, Opts{{"Trim", "[\"..invalid.path\"]"}} ) );
  }
}


TEST_CASE_FIXTURE(Fixture, "constructing Explode filter")
{
  const auto type = FilterFactory::Type{"Explode"};

  SUBCASE("valid")
  {
    CHECK( ff_.build( type, name_, Opts{{"Path", ".foo.bar"}} )->type().value_ == type.name_ );
  }

  SUBCASE("missing argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{} ), FilterFactory::MissingOption );
  }

  SUBCASE("unknown argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{{"Path", ".foo.bar"}, {"foo", "bar"}} ), FilterFactory::UnknownOption );
  }

  SUBCASE("invalid argument")
  {
    CHECK_THROWS( ff_.build( type, name_, Opts{{"Path", ".invalid..path"}} ) );
  }
}


TEST_CASE_FIXTURE(Fixture, "constructing Grep filter")
{
  const auto type = FilterFactory::Type{"Grep"};

  SUBCASE("valid")
  {
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Key"},
                                         {"Case", "Sensitive"},
                                         {"Trim", "False"},
                                         {"Search", "Regular"}
                                       } )->type().value_ == type.name_ );
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Value"},
                                         {"Case", "Sensitive"},
                                         {"Trim", "False"},
                                         {"Search", "Regular"}
                                       } )->type().value_ == type.name_ );
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Key"},
                                         {"Case", "Insensitive"},
                                         {"Trim", "False"},
                                         {"Search", "Regular"}
                                       } )->type().value_ == type.name_ );
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Key"},
                                         {"Case", "Sensitive"},
                                         {"Trim", "False"},
                                         {"Search", "Inverse"}
                                       } )->type().value_ == type.name_ );
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Key"},
                                         {"Case", "Sensitive"},
                                         {"Trim", "True"},
                                         {"Search", "Regular"}
                                       } )->type().value_ == type.name_ );
  }

  SUBCASE("missing argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"},
                                                   {"Trim", "True"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"},
                                                   {"Trim", "True"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Case", "Sensitive"},
                                                   {"Trim", "True"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Trim", "True"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Trim", "True"},
                                                   {"Case", "Sensitive"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
  }

  SUBCASE("unknown argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"},
                                                   {"Trim", "True"},
                                                   {"Search", "Regular"},
                                                   {"foo", "bar"}
                                                 } ), FilterFactory::UnknownOption );
  }

  SUBCASE("invalid argument")
  {
    CHECK_THROWS( ff_.build( type, name_, Opts{
                                                {"Path", ".foo.bar"},
                                                {"regex", "** invalid regex **"},
                                                {"Compare", "Key"},
                                                {"Case", "Sensitive"},
                                                {"Trim", "True"},
                                                {"Search", "Regular"}
                                              } ) );
    CHECK_THROWS( ff_.build( type, name_, Opts{
                                                {"Path", ".invalid..path"},
                                                {"regex", ".*"},
                                                {"Compare", "Key"},
                                                {"Case", "Sensitive"},
                                                {"Trim", "True"},
                                                {"Search", "Regular"}
                                               } ) );
    CHECK_THROWS( ff_.build( type, name_, Opts{
                                                {"Path", ".foo.bar"},
                                                {"regex", "a.*"},
                                                {"Compare", "Key"},
                                                {"Case", "Sensitive"},
                                                {"Trim", "invalid trim value"},
                                                {"Search", "Regular"}
                                              } ) );
  }
}


TEST_CASE_FIXTURE(Fixture, "constructing BinarySplit filter")
{
  const auto type = FilterFactory::Type{"BinarySplit"};

  SUBCASE("valid")
  {
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Key"},
                                         {"Case", "Sensitive"},
                                         {"Trim", "False"},
                                         {"Search", "Regular"}
                                       } )->type().value_ == type.name_ );
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Value"},
                                         {"Case", "Sensitive"},
                                         {"Trim", "False"},
                                         {"Search", "Regular"}
                                       } )->type().value_ == type.name_ );
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Key"},
                                         {"Case", "Insensitive"},
                                         {"Trim", "False"},
                                         {"Search", "Regular"}
                                       } )->type().value_ == type.name_ );
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Key"},
                                         {"Case", "Sensitive"},
                                         {"Trim", "False"},
                                         {"Search", "Inverse"}
                                       } )->type().value_ == type.name_ );
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Key"},
                                         {"Case", "Sensitive"},
                                         {"Trim", "True"},
                                         {"Search", "Regular"}
                                       } )->type().value_ == type.name_ );
  }

  SUBCASE("missing argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"},
                                                   {"Trim", "True"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"},
                                                   {"Trim", "True"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Case", "Sensitive"},
                                                   {"Trim", "True"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Trim", "True"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Trim", "True"},
                                                   {"Case", "Sensitive"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
  }

  SUBCASE("unknown argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"},
                                                   {"Trim", "True"},
                                                   {"Search", "Regular"},
                                                   {"foo", "bar"}
                                                 } ), FilterFactory::UnknownOption );
  }

  SUBCASE("invalid argument")
  {
    CHECK_THROWS( ff_.build( type, name_, Opts{
                                                {"Path", ".foo.bar"},
                                                {"regex", "** invalid regex **"},
                                                {"Compare", "Key"},
                                                {"Case", "Sensitive"},
                                                {"Trim", "True"},
                                                {"Search", "Regular"}
                                              } ) );
    CHECK_THROWS( ff_.build( type, name_, Opts{
                                                {"Path", ".invalid..path"},
                                                {"regex", ".*"},
                                                {"Compare", "Key"},
                                                {"Case", "Sensitive"},
                                                {"Trim", "True"},
                                                {"Search", "Regular"}
                                               } ) );
    CHECK_THROWS( ff_.build( type, name_, Opts{
                                                {"Path", ".foo.bar"},
                                                {"regex", "a.*"},
                                                {"Compare", "Key"},
                                                {"Case", "Sensitive"},
                                                {"Trim", "invalid trim value"},
                                                {"Search", "Regular"}
                                              } ) );
  }

  SUBCASE("checking children")
  {
    const auto bs = ff_.build( type, name_, Opts{
            {"Path", ".foo.bar"},
            {"regex", "a.*"},
            {"Compare", "Key"},
            {"Case", "Sensitive"},
            {"Trim", "False"},
            {"Search", "Regular"}
        } );
    CHECK( bs->name() == name_ );
    const auto children = bs->children();
    REQUIRE( children.size() == 2 );
    CHECK( children[0]->type() == Node::Type{"Grep"} );
    CHECK( children[0]->name() == Node::Name{"<matched>"} );
    CHECK( children[1]->type() == Node::Type{"AcceptAll"} );
    CHECK( children[1]->name() == Node::Name{"<unmatched>"} );
  }
}


TEST_CASE_FIXTURE(Fixture, "constructing From filter")
{
  const auto type = FilterFactory::Type{"From"};

  SUBCASE("valid")
  {
    CHECK( ff_.build( type, name_, Opts{{"Edge", "xxx/42"}} )->type().value_ == type.name_ );
  }

  SUBCASE("missing argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{} ), FilterFactory::MissingOption );
  }

  SUBCASE("unknown argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{{"Edge", "xxx/42"}, {"foo", "bar"}} ), FilterFactory::UnknownOption );
  }

  SUBCASE("invalid argument")
  {
    CHECK_THROWS( ff_.build( type, name_, Opts{{"Edge", "xxx/43-not-a-number"}} ) );
  }
}


TEST_CASE_FIXTURE(Fixture, "constructing To filter")
{
  const auto type = FilterFactory::Type{"To"};

  SUBCASE("valid")
  {
    CHECK( ff_.build( type, name_, Opts{{"Edge", "xxx/42"}} )->type().value_ == type.name_ );
  }

  SUBCASE("missing argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{} ), FilterFactory::MissingOption );
  }

  SUBCASE("unknown argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{{"Edge", "xxx/42"}, {"foo", "bar"}} ), FilterFactory::UnknownOption );
  }

  SUBCASE("invalid argument")
  {
    CHECK_THROWS( ff_.build( type, name_, Opts{{"Edge", "xxx/43-not-a-number"}} ) );
  }
}

}
}
