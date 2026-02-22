#include "zolal/Manifest.hpp"
#include <filesystem>
#include <iostream>
#include <ostream>
#include <toml++/toml.h>

namespace Z::Manifest {
std::optional<Manifest> Manifest::load(const std::string &path) {
    if (!std::filesystem::exists(path)) {
        std::cerr << "Manifest file not found" << path << std::endl;
        return std::nullopt;
    }

    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
    } catch (const toml::parse_error &err) {
        std::cerr << "Toml Parse Error: " << err << std::endl;
        return std::nullopt;
    }

    Manifest manifest;
    if (auto project = tbl["project"].as_table()) {
        manifest.project.name = project->at("name").value_or<std::string>("");

        manifest.project.version = project->at("version").value_or<std::string>("");

        if (auto entry = (*project)["entry"].as_string()) {
            manifest.project.entry = entry->get();
        }
    } else {
        std::cerr << "Missing [project] section in manifest\n";
        return std::nullopt;
    }

    if (auto build = tbl["build"].as_table()) {
        manifest.build.output = (*build)["output"].value_or<std::string>("");

        if (auto target_arr = (*build)["target"].as_array()) {
            auto parse_lang = [](const std::string &s) -> std::optional<OutputLang> {
                if (s == "native") return OutputLang::Native;

                if (s == "js") return OutputLang::Js;

                if (s == "python") return OutputLang::Python;

                return std::nullopt;
            };

            if (target_arr->size() == 1) {
                if (auto val = (*target_arr)[0].as_string()) {
                    if (auto lang = parse_lang(val->get())) {
                        manifest.build.lang = *lang;
                    } else {
                        manifest.build.target = val->get();
                    }
                }
            }
            if (target_arr->size() >= 2) {
                if (auto triplet = (*target_arr)[0].as_string())
                    manifest.build.target = triplet->get();
                if (auto lang_node = (*target_arr)[1].as_string()) {
                    if (auto lang = parse_lang(lang_node->get()))
                        manifest.build.lang = *lang;
                    else {
                        std::cerr << "Unknown output language: " << lang_node->get() << std::endl;
                        return std::nullopt;
                    }
                }
            }
        }
    }

    if (auto sources = tbl["sources"].as_table()) {
        if (auto files = sources->at("files").as_array()) {
            for (auto &&elem : *files) {
                if (auto str = elem.as_string()) {
                    manifest.sources.push_back(str->get());
                }
            }
        }
    }

    if (auto deps = tbl["dependencies"].as_table()) {
        for (auto &&[key, value] : *deps) {
            Dependency dep;
            dep.name = std::string(key);
            if (auto dep_table = value.as_table()) {
                dep.path = dep_table->at("path").value_or<std::string>("");
                manifest.dependencies.push_back(dep);
            }
        }
    }

    if (auto native = tbl["native"].as_table()) {
        if (auto libs = native->at("libraries").as_array()) {
            for (auto &&elem : *libs) {
                if (auto str = elem.as_string()) {
                    manifest.native.libraries.push_back(str->get());
                }
            }
        }
        if (auto lib_paths = native->at("lib_paths").as_array()) {
            for (auto &&elem : *lib_paths) {
                if (auto str = elem.as_string()) {
                    manifest.native.lib_paths.push_back(str->get());
                }
            }
        }
    }

    return manifest;
}
} // namespace Z::Manifest
