// Copyright 2024 Yun-Tse, Yang.
// Distributed under the Boost Software License, Version 1.0.
#pragma once
#include <map>
#include <string>
#include <string_view>
class CommandLine {
public:
    CommandLine(int argc, const char** argv) { Parse(argc, argv); }
    bool HasSwitch(const std::string& s) {
        return switches_.count(s);
    }
    std::string GetSwitchValue(const std::string& s) const {
        auto iter = switches_.find(s);
        if (iter == switches_.end())
            return {};
        return iter->second;
    }
    const auto &GetArgs() const {
        return args_;
    }
    const auto& Program() const { return prog_; }
private:
    void Parse(int argc, const char** argv) {
        prog_ = argv[0];
        for (int i = 1; i < argc; ++i) {
            std::string item = argv[i];
            if (IsSwitch(item)) {
                ParseSwitch(item);
            } else {
                args_.push_back(std::move(item));
            }
        }
    }
    bool IsSwitch(const std::string& item) const {
        return item.size() > 2 && item[0] == '-' && item[1] == '-';
    }
    void ParseSwitch(std::string_view item) {
        item = item.substr(2);
        auto assign_pos = item.find('=');
        std::string_view key = item.substr(0, assign_pos);
        std::string_view val;
        if (assign_pos != std::string::npos) {
            val = item.substr(++assign_pos);
        }
        switches_.emplace(key, val);
    }
    std::string prog_;
    std::map<std::string, std::string> switches_;
    std::vector<std::string> args_;
};
