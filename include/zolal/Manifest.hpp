#pragma once
#include <optional>
#include <string>
#include <vector>

namespace Z::Manifest {
enum class OutputLang { Native, Js, Python };

struct ProjectInfo {
    std::string                name;
    std::string                version;
    std::optional<std::string> entry; // present = binary, absent = lib
};

struct BuildConfig {
    std::string target = "x86_64-unknown-linux-gnu"; // machine triplets like
                                                     // "x86_64-unknown-linux-gnu"
    OutputLang  lang = OutputLang::Native;
    std::string output; // output file name
};

struct NativeConfig {
    std::vector<std::string> libraries;
    std::vector<std::string> lib_paths;
};

struct Dependency {
    std::string name;
    std::string path;
};

struct Manifest {
    ProjectInfo              project;
    BuildConfig              build;
    std::vector<std::string> sources;
    std::vector<Dependency>  dependencies;
    NativeConfig             native;

    static std::optional<Manifest> load(const std::string &path);
};
} // namespace Z::Manifest
