#include <zolal/Config.hpp>
#include <zolal/Manifest.hpp>
#include <zolal/Preprocessor.hpp>

#include <argparse/argparse.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

static std::string read_file(const std::string &path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";

    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static bool write_file(const std::string &path, const std::string &content) {
    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << content;
    return true;
}

int main(int argc, char *argv[]) {
    // TODO: get this from argv[0], split slashes and get last.
    argparse::ArgumentParser program(
        "zolal",
        std::format("{}.{}.{}", ZOLAL_VERSION_MAJOR, ZOLAL_VERSION_MINOR, ZOLAL_VERSION_PATCH));

    program.add_argument("-S", "--source-dir")
        .help("Place where source code and manifest.zl resides.")
        .remaining()
        .metavar("source-dir");
    program.add_argument("-B", "--output-dir")
        .help("Output directory for zolal builds.")
        .remaining()
        .metavar("output-dir");

    if (argc == 1) {
        std::cout << program;
        std::exit(0);
    }

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    std::string manifest_source_directory = ".";
    if (program.is_used("--source-dir")) {
        manifest_source_directory = program.get<std::string>("--source-dir");
        if (!std::filesystem::exists(manifest_source_directory)) {
            std::cerr << "Source directory does not exist.\n";
            return 1;
        }
    }

    std::filesystem::path manifest_path =
        std::filesystem::path(manifest_source_directory) / "manifest.toml";
    if (!std::filesystem::exists(manifest_path)) {
        std::cerr << "No manifest.toml found in current directory.\n";
        return 1;
    }

    auto manifest = Zolal::Manifest::load(manifest_path.string());
    if (!manifest) {
        std::cerr << "Failed to load manifest\n";
        return 1;
    }

    Zolal::Preprocessor pp(*manifest);

    std::string out_dir = ".zpp";

    if (program.is_used("--output-dir")) {
        out_dir = program.get<std::string>("--output-dir");
    }

    bool had_errors      = false;
    int  files_processed = 0;

    // processes project files
    for (const auto &source_path : manifest->sources) {
        if (!std::filesystem::exists(std::filesystem::path(manifest_source_directory) /
                                     source_path)) {
            std::cerr << "Source file not found " << source_path << std::endl;
            had_errors = true;
            continue;
        }

        std::string content =
            read_file((std::filesystem::path(manifest_source_directory) / source_path).string());
        auto result = pp.process(content);

        if (!result.ok()) {
            for (const auto &err : result.errors) {
                std::cerr << source_path << ":" << err.line << ": " << err.message << std::endl;
                had_errors = true;
                continue;
            }
        }
        std::string output_path = out_dir + "/" + source_path;
        if (!write_file(output_path, result.output)) {
            std::cerr << "Failed to write: " << output_path << std::endl;
            had_errors = true;
            continue;
        }

        files_processed++;
    }

    // and then dependencies
    for (const auto &dep : manifest->dependencies) {
        if (dep.path.empty()) {
            continue;
        }

        std::string dep_manifest_path = dep.path + "/manifest.toml";
        if (!std::filesystem::exists(dep_manifest_path)) {
            std::cerr << "Dependency '" << dep.name << "' manifest not found" << dep_manifest_path
                      << std::endl;
            had_errors = true;
            continue;
        }

        auto dep_manifest = Zolal::Manifest::load(dep_manifest_path);
        if (!dep_manifest) {
            std::cerr << "Failed to load dependency manifest: " << dep.name << "\n";
            had_errors = true;
            continue;
        }

        // deps must be libs
        if (dep_manifest->project.entry.has_value()) {
            std::cerr << "Dependency '" << dep.name
                      << "' has an entry point and must be a library.\n";
            had_errors = true;
            continue;
        }
        for (const auto &dep_source : dep_manifest->sources) {
            std::string full_path = dep.path + "/" + dep_source;
            if (!std::filesystem::exists(full_path)) {
                std::cerr << "Dependency source not found: " << full_path << "\n";
                had_errors = true;
                continue;
            }

            std::string content = read_file(full_path);
            auto        result  = pp.process(content);

            if (!result.ok()) {
                for (const auto &err : result.errors) {
                    std::cerr << full_path << ":" << err.line << ": " << err.message << "\n";
                }
                had_errors = true;
                continue;
            }

            std::string out_path = out_dir + "/" + full_path;
            if (!write_file(out_path, result.output)) {
                std::cerr << "Failed to write: " << out_path << "\n";
                had_errors = true;
                continue;
            }

            files_processed++;
        }
    }
    if (had_errors) {
        std::cerr << "Preprocessing completed with errors.\n";
        return 1;
    }

    std::cout << "Preprocessed " << files_processed << " file(s) → " << out_dir << "/\n";
    return 0;
}