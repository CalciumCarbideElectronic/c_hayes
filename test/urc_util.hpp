#ifndef _URC_UTIL_HPP
#define _URC_UTIL_HPP
extern "C" {
#include "semaphore.h"
}
#include <stdio.h>
#include <regex>
#include <stack>
#include <string>
#include <unordered_map>

class URCHelper {
   public:
    static std::unordered_map<std::string, URCHelper *> objs;
    URCHelper(std::string urc_tag) : mTag(urc_tag), mHitTimes(0) {
        URCHelper::objs.insert(std::make_pair(urc_tag, this));
    };

    ~URCHelper() { URCHelper::objs.erase(mTag); }

    static void hook(const char *buf, const char *objkey) {
        auto recv = std::string(buf);
        std::string key = parsekey(buf);
        if (key.empty()) key = std::string("plain");
        auto obj = get_obj(objkey);
        if (key == std::string(objkey)) obj->mHitTimes += 1;
    }

    static uint64_t hit_times(const char *objkey) {
        auto obj = get_obj(objkey);
        if (obj == NULL) return 0;
        return obj->mHitTimes;
    }
    uint64_t hit_times() { return mHitTimes; }

    static URCHelper *get_obj(const char *objkey) {
        auto obj = URCHelper::objs.find(std::string(objkey));
        if (obj == URCHelper::objs.end()) return NULL;
        return obj->second;
    }

    std::stack<std::string> received_stack;

   private:
    uint64_t mHitTimes;
    std::string mTag;
    static std::string parsekey(const char *buf) {
        const std::regex re("^\\+(.+):");
        std::smatch s_match;
        auto buf_string = std::string(buf);

        std::regex_search(buf_string, s_match, re);
        if (s_match.size() < 2) return std::string();
        auto sub_m = s_match[1];
        return sub_m.str();
    }
};

std::unordered_map<std::string, URCHelper *> URCHelper::objs;

#endif
