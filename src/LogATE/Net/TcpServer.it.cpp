#include <doctest/doctest.h>
#include "LogATE/Net/TcpServer.hpp"
#include "LogATE/Net/TcpClient.hpp"
#include "LogATE/Net/TcpRawClient.hpp"
#include "LogATE/TestPrints.ut.hpp"
#include "But/Optional.hpp"

using LogATE::Log;
using LogATE::Net::Port;
using LogATE::Net::TcpServer;
using LogATE::Net::TcpClient;
using LogATE::Net::TcpRawClient;

namespace
{
TEST_SUITE("Net::TcpServer")
{

struct Fixture
{
  nlohmann::json str2json(std::string const& in) const { return nlohmann::json::parse(in); }

  const LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  const Port port_{4242};
  const std::string host_{"127.0.0.1"};
  const nlohmann::json log1_{ str2json( R"({ "foo": "bar" })" ) };
  const nlohmann::json log2_{ str2json( R"({ "xxx": "yyy" })" ) };
};

TEST_CASE_FIXTURE(Fixture, "client fails to connect to a closed socket")
{
  CHECK_THROWS_WITH( (TcpClient{"127.6.6.6", Port{6666}}), "Connection refused" );
}

TEST_CASE_FIXTURE(Fixture, "server can handle one client")
{
  TcpServer s{workers_, port_};
  TcpClient c{host_, port_};
  CHECK( s.errors() == 0 );

  c.write(log1_);
  const auto log = s.readNextLog();
  CHECK( s.errors() == 0 );
  REQUIRE(log);
  CHECK( log->json() == log1_ );
}

TEST_CASE_FIXTURE(Fixture, "server can handle one client and multiple messages")
{
  TcpServer s{workers_, port_};
  TcpClient c{host_, port_};

  for(auto const& json: {log1_, log2_})
  {
    c.write(json);
    const auto log = s.readNextLog();
    CHECK( s.errors() == 0 );
    REQUIRE(log);
    CHECK( log->json() == json );
  }
}

TEST_CASE_FIXTURE(Fixture, "server does not disconnect on parse errors")
{
  TcpServer s{workers_, port_};
  TcpRawClient rc{host_, port_};

  CHECK( s.errors() == 0 );
  rc.write("not a valid json\n");
  rc.write( log1_.dump() );

  const auto log = s.readNextLog();
  CHECK( s.errors() == 12 );
  REQUIRE(log);
  CHECK( log->json() == log1_ );
}

TEST_CASE_FIXTURE(Fixture, "server can handle json spread through multiple lines")
{
  TcpServer s{workers_, port_};
  TcpRawClient rc{host_, port_};

  const auto jsonStr = std::string{R"( {
                                         "one": 1,
                                         "two": []
                                       })"};
  rc.write(jsonStr);

  const auto log = s.readNextLog();
  CHECK( s.errors() == 0 );
  REQUIRE(log);
  CHECK( log->json() == nlohmann::json::parse(jsonStr) );
}

TEST_CASE_FIXTURE(Fixture, "server can handle jsons attached to next other")
{
  TcpServer s{workers_, port_};
  TcpRawClient rc{host_, port_};

  const auto jsonStr = std::string{R"({"foo":"bar"})"};
  const auto count = 2;
  for(auto i=0; i<count; ++i)
    rc.write(jsonStr);

  for(auto i=0; i<count; ++i)
  {
    const auto log = s.readNextLog();
    CHECK( s.errors() == 0 );
    REQUIRE(log);
    CHECK( log->json() == nlohmann::json::parse(jsonStr) );
  }
}

TEST_CASE_FIXTURE(Fixture, "server gets back to accepting connections once current client disconnects")
{
  TcpServer s{workers_, port_};

  for(auto i=0; i<2; ++i)
  {
    TcpClient c{host_, port_};
    c.write(log1_);
    const auto log = s.readNextLog();
    CHECK( s.errors() == i );
    REQUIRE(log);
    CHECK( log->json() == log1_ );
  }
}

TEST_CASE_FIXTURE(Fixture, "server can be stopped even when client is still connected")
{
  But::Optional<TcpRawClient> rc;
  TcpServer s{workers_, port_};
  rc.emplace(host_, port_);
}

TEST_CASE_FIXTURE(Fixture, "null jsons are dropped")
{
  TcpServer s{workers_, port_};
  TcpClient r{host_, port_};

  CHECK( s.errors() == 0 );
  const nlohmann::json null;
  REQUIRE( null.is_null() );
  r.write(null);
  r.write(log1_);

  const auto log = s.readNextLog();
  CHECK( s.errors() == 0 );
  REQUIRE(log);
  CHECK( log->json() == log1_ );
}

}
}
