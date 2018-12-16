#include "LogATE/SequenceNumber.hpp"

namespace LogATE
{

namespace
{
auto nextFreeSN()
{
  static std::atomic<uint64_t> nextFree{0};
  return nextFree++;
}
}

SequenceNumber::SequenceNumber()
  value_{ nextFreeSN() }
{ }

}