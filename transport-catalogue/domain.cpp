#include "domain.h"

namespace domain {

std::string_view Commands::AddId(const std::string& id) {
    ids_.push_front(id);
    return {ids_.front().begin(), ids_.front().end()};
}
    
}
