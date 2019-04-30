#pragma once
#include "LogATE/Tree/Node.hpp"

namespace LogATE::Tree
{

class SimpleNode: public Node
{
public:
  void insert(Log const& log) override final;
  Children children() const override final;
  NodeShPtr add(NodePtr node) override final;
  bool remove(NodeShPtr node) override final;

protected:
  SimpleNode(Utils::WorkerThreadsShPtr workers, Type type, Name name, TrimFields trimFields):
    Node{ std::move(workers), std::move(type), std::move(name), std::move(trimFields) }
  { }

private:
  virtual bool matches(Log const& log) const = 0;

  void insertToChildren(Log const& log);
  void insertToChild(NodeShPtr const& child, Log const& log);
  void passAllLogsToChild(NodeShPtr const& child);

  using Lock = std::lock_guard<std::mutex>;
  mutable std::mutex mutex_;
  Children children_;
};

}
