#pragma once
#include "LogATE/Log.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <sstream>

namespace CursATE::Screen::LogDisplay
{

class OrderedPrettyPrint final
{
public:
  struct SilentTags final
  {
    std::vector<std::string> tags_;
  };
  struct PriorityTags final
  {
    std::vector<std::string> tags_;
  };

  OrderedPrettyPrint() = default;
  explicit OrderedPrettyPrint(PriorityTags const& priorityTags): OrderedPrettyPrint{ priorityTags, {} } { }
  explicit OrderedPrettyPrint(SilentTags const& silentTags): OrderedPrettyPrint{ {}, silentTags } { }
  OrderedPrettyPrint(SilentTags const& silentTags, PriorityTags const& priorityTags): OrderedPrettyPrint{ priorityTags, silentTags } { }
  OrderedPrettyPrint(PriorityTags const& priorityTags, SilentTags const& silentTags);

  std::string operator()(LogATE::Log const& in) const;

private:
  void printNode(std::stringstream& ss, std::string const& key, nlohmann::json const& value) const;
  void constructString(std::stringstream& ss, nlohmann::json const& in) const;
  bool isSilent(std::string const& tag) const;

  const std::unordered_set<std::string> silentTags_{};
  const PriorityTags priorityTags_{};
};

}
