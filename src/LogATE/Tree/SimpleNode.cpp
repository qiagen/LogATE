#include "LogATE/Tree/SimpleNode.hpp"

namespace LogATE::Tree
{

void SimpleNode::insert(Log const& log)
{
  if( not matches(log) )
    return;
  logs().withLock()->insert(log);
  insertToChildren(log);
}

SimpleNode::Children SimpleNode::children() const
{
  const Lock lock{mutex_};
  return children_;
}

void SimpleNode::add(NodePtr node)
{
  auto shared = NodeShPtr{std::move(node)};
  {
    const Lock lock{mutex_};
    children_.push_back(shared);
  }
  passAllLogsToChild(shared);
}

void SimpleNode::insertToChildren(Log const& log)
{
  for(auto c: children_)
    insertToChild(c, log);
}


void SimpleNode::insertToChild(NodeShPtr const& child, Log const& log)
{
  try
  {
    child->insert(log);
  }
  catch(...)
  {
    // whatever...
  }
}


namespace
{
auto copyAll(Logs const& logs)
{
  const auto& locked = logs.withLock();
  return std::vector<Log>{ locked->begin(), locked->end() };

}
}

void SimpleNode::passAllLogsToChild(NodeShPtr const& child)
{
  for(auto const& log: copyAll(logs_))
    insertToChild(child, log);
}

}