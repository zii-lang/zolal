#include <argparse/argparse.hpp>
#include <zolal/Config.hpp>

int main(int argc, char *argv[]) {
    // TODO: get this from argv[0], split slashes and get last.
    argparse::ArgumentParser program(
        "zolal",
        std::format("{}.{}.{}", ZOLAL_VERSION_MAJOR, ZOLAL_VERSION_MINOR, ZOLAL_VERSION_PATCH));

    program.add_argument("-S", "--source-dir")
        .help("Place where source code and manifest.zl resides.")
        .default_value(".")
        .nargs(1);
    program.add_argument("-B", "--output-dir")
        .help("Output directory for zolal builds.")
        .default_value("zout")
        .nargs(1);

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

    std::string manifest_source_directory = program.get<std::string>("--source-dir");
    if (!std::filesystem::exists(manifest_source_directory)) {
        std::cerr << "Source directory does not exist.\n";
        return 1;
    }

    std::filesystem::path manifest_path =
        std::filesystem::path(manifest_source_directory) / "manifest.toml";
    if (!std::filesystem::exists(manifest_path)) {
        std::cerr << "No manifest.toml found in current directory.\n";
        return 1;
    }

    return 0;
}
