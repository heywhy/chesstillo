#include <string_view>
#include <unordered_map>

#include <uci/types.hpp>

namespace uci {
std::unordered_map<std::string_view, Status> kStatus = {
    {"ok", Status::OK},
    {"error", Status::ERROR},
    {"checking", Status::CHECKING}};
}
