#include "urc_util.hpp"

std::unordered_map<std::string, URCHelper*> URCHelper::objs;
URCHelper::URCHelper(std::string urc_tag) : mTag(urc_tag), mHitTimes(0) {
    URCHelper::objs.insert(std::make_pair(urc_tag, this));
}

const std::stack<std::string>& URCHelper::access_recv() {
    return received_stack;
}
