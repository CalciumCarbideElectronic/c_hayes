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

    static void hook(const char *buf, const char *objkey) {
        printf("%s\n", buf);
        printf("%s\n", objkey);
        auto recv = std::string(buf);
        const std::regex re("^\\+(.+):");

        auto buf_string = std::string(buf);
        std::smatch s_match;
        std::regex_search(buf_string, s_match, re);

        std::string key;
        printf("size:%d\n", s_match.size());

        if (s_match.size() < 2) {
            key = std::string("plain");
        } else {
            auto sub_m = s_match[1];
            key = sub_m.str();
        };

        auto obj = get_obj(objkey);
        obj->received_stack.push(recv);
        printf("key: %s objkey:%s,obj:%p\n", key.c_str(), objkey, obj);

        if (key == objkey) {
            printf("objkey: %s, times: %u\n", objkey, obj->mHitTimes);
            obj->mHitTimes += 1;
        }
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
