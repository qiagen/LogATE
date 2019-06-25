#pragma once
#include <initializer_list>
#include <But/System/Descriptor.hpp>
#include "LogATE/Net/Exception.hpp"

namespace LogATE::Net
{

BUT_DEFINE_EXCEPTION(EpollFailed, Exception, "epoll() failed");

enum class ReadyFor
{
    Read = 1,
    Write = 2,
    ReadWrite = 1 | 2
};

struct ObservedEntry final
{
    But::System::Descriptor* fd;
    ReadyFor readyFor;
};

But::System::Descriptor* epoll(std::initializer_list<ObservedEntry> fds);

}
