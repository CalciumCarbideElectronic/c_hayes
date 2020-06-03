#ifndef _URC_UTIL_HPP
#define _URC_UTIL_HPP
#include <stdio.h>
#include <regex>
#include <stack>
#include <string>
#include <unordered_map>

class URCHelper {
   public:
    static std::unordered_map<std::string, URCHelper *> objs;
    URCHelper(std::string urc_tag);
    static void reset() { objs.clear(); }
    ~URCHelper() { URCHelper::reset(); }

    static std::string parsekey(const char *buf) {
        const std::regex re("^\\+(.+):");
        std::smatch s_match;
        auto buf_string = std::string(buf);

        std::regex_search(buf_string, s_match, re);
        if (s_match.size() < 2) return std::string();
        auto sub_m = s_match[1];
        return sub_m.str();
    }

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

    static URCHelper *get_obj(const char *objkey) {
        auto obj = URCHelper::objs.find(std::string(objkey));
        if (obj == URCHelper::objs.end()) return NULL;
        return obj->second;
    }

    const std::stack<std::string> &access_recv();
    std::stack<std::string> received_stack;
    uint64_t mHitTimes;

   private:
    std::string mTag;
};

#endif
