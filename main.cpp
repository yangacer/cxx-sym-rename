// Copyright 2024 Yun-Tse, Yang.
// Distributed under the Boost Software License, Version 1.0.
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <functional>
#include <sstream>
#include "demangle.h"
#include "command_line.h"

struct Meta {
    std::string demangled;
    std::vector<std::string> source_names;
};

size_t Replace(std::string* str, const std::string& old_pattern,
               const std::string& new_pattern, size_t start_pos) {
    size_t pos = str->find(old_pattern);
    if (std::string::npos != pos) {
        str->replace(pos, old_pattern.size(), new_pattern);
        pos += new_pattern.size();
    }
    return pos;
}

void Usage(const CommandLine& cmdl) {
    std::cerr << cmdl.Program() << " [--help] [--prefix=<string>] [--salt=<string>]" << std::endl;
    std::cerr << "where" << std::endl;
    std::cerr << "  --help              Print this message." << std::endl;
    std::cerr << "  --prefix=<string>   Prepend <string> to all renamed symbols." << std::endl;
    std::cerr << "  --salt=<string>     Add <string> as salt of renaming encryption." << std::endl;
}

int main(int argc, const char** argv) {
    CommandLine cmdl(argc, argv);

    if (cmdl.HasSwitch("help")) {
        Usage(cmdl);
        return 0;
    }

    std::string prefix = "CXXSR_";
    std::string salt;

    if (cmdl.HasSwitch("prefix")) {
        prefix = cmdl.GetSwitchValue("prefix");
    }

    if (cmdl.HasSwitch("salt")) {
        salt = cmdl.GetSwitchValue("salt");
    }

    std::vector<char> outbuf(2048);
    std::map<std::string, Meta> symbol_map;
    std::set<std::string> all_source_names;
    std::map<std::string, std::string> encode_map;

    while(!std::cin.eof()) {
        std::string line;
        std::getline(std::cin, line);
        if (line.empty())
            continue;
        std::vector<std::string> source_names;
        if (Demangle(line.c_str(), outbuf.data(), outbuf.size(), &source_names)) {
            for (const auto &n : source_names) {
                if (n.find("__") == 0)
                    continue;
                all_source_names.insert(n);
            }
            symbol_map[line] = Meta {{outbuf.data()}, std::move(source_names)};
        }
    }

    std::set<size_t> seen_hash;
    std::hash<std::string> hasher;
    for (auto &n : all_source_names) {
        auto hv = hasher(n + salt);
        int key_range = 100 * all_source_names.size();
        while (seen_hash.count(hv % key_range)) {
            key_range *= 2;
        }
        hv %= key_range;
        seen_hash.insert(hv);
        std::stringstream ss;
        ss << prefix << std::hex << hv;
        auto encoded = ss.str();
        ss.str("");
        ss.clear();
        ss << encoded.size() << encoded;
        encoded = ss.str();
        encode_map[n] = encoded;
    }

    for (const auto &kv : symbol_map) {
        std::cout << "# " << kv.second.demangled << std::endl;
        std::string replaced = kv.first;
        size_t start_pos = 0;
        for (const auto& n : kv.second.source_names) {
            start_pos = Replace(&replaced, n, encode_map[n], start_pos);
        }
        std::cout << kv.first << " " << replaced << std::endl;
    }

    return 0;
}
