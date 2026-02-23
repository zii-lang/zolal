#include "zolal/DirectiveLexer.hpp"
#include "zolal/Manifest.hpp"
#include <sstream>
#include <type_traits>
#include <zolal/Preprocessor.hpp>

namespace Zolal {

    void Preprocessor::parse_triplets() {
        std::vector<std::string> parts;
        std::istringstream       ss(manifest.build.target);
        std::string              part;
        while (std::getline(ss, part, '-')) {
            parts.push_back(part);
        }

        std::string arch;
        std::string vendor;
        std::string os;
        std::string env;

        if (parts.size() == 4) {
            arch   = parts[0];
            vendor = parts[1];
            os     = parts[2];
            env    = parts[3];
        } else if (parts.size() == 3) {
            arch   = parts[0];
            vendor = parts[1];
            os     = parts[2];
        } else if (parts.size() == 2) {
            arch = parts[0];
            os   = parts[1];
        } else if (parts.size() == 1) {
            arch = parts[0];
        }

        if (!arch.empty()) derived["arch"] = arch;
        if (!vendor.empty()) derived["vendor"] = vendor;
        if (!os.empty()) derived["os"] = os;
        if (!env.empty()) derived["env"] = env;

        if (arch == "x86_64" || arch == "aarch64" || arch == "riscv64" || arch == "ppc64" ||
            arch == "ppc64le" || arch == "s390x")
            derived["pointer_width"] = "64";
        else
            derived["pointer_width"] = "32";

        if (arch == "mips" || arch == "mips64" || arch == "sparc" || arch == "sparc64" ||
            arch == "ppc" || arch == "ppc64" || arch == "s390x")
            derived["endian"] = "big";
        else
            derived["endian"] = "little";

        if (os == "linux" || os == "android" || os == "darwin" || os == "macos" ||
            os == "freebsd" || os == "openbsd" || os == "netbsd")
            derived["os_family"] = "unix";
        else if (os == "windows" || os == "win32")
            derived["os_family"] = "windows";

        if (arch == "x86_64") {
            derived["has_sse2"] = "1";
            derived["has_avx"]  = "1";
        } else if (arch == "aarch64") {
            derived["has_neon"] = "1";
        }

        // TODO: should probably support more vendors
        if (arch == "nvptx64")
            derived["gpu_vendor"] = "nvidia";
        else if (arch == "amdgcn")
            derived["gpu_vendor"] = "amd";
    }

    std::string Preprocessor::resolve_var(const std::string &name) const {
        if (name == "target") return manifest.build.target;
        if (name == "project_name") return manifest.project.name;
        if (name == "project_version") return manifest.project.version;
        if (name == "output") return manifest.build.output;
        if (name == "lang") {
            switch (manifest.build.lang) {
                case Zolal::OutputLang::Python:
                    return "python";
                case Zolal::OutputLang::Js:
                    return "js";
                case Zolal::OutputLang::Native:
                    return "native";
            }
        }

        auto it = derived.find(name);
        if (it != derived.end()) return it->second;

        return "";
    }

    bool Preprocessor::has_var(const std::string &name) const {
        // direct manifest fields that are always present
        if (name == "target" || name == "project_name" || name == "project_version" ||
            name == "output" || name == "lang")
            return true;

        if (name == "is_library") return !manifest.project.entry.has_value();
        if (name == "is_binary") return manifest.project.entry.has_value();

        return derived.contains(name);
    }

    // example:
    // for #if arch == "x86_64" && os_family == "unix",
    // it recursively evaluates: resolve arch → "x86_64",
    // compare with "x86_64" → true.
    // Resolve os_family → "unix", compare → true. true && true → true.
    bool Preprocessor::evaluate(const std::shared_ptr<CondExpr> &expr) const {
        if (!expr) return false;

        // pattern matches on the variant
        return std::visit(
            [&](const auto &val) -> bool {
                using T = std::decay_t<decltype(val)>;

                if constexpr (std::is_same_v<T, CondLiteral>) {
                    // it its a name but not a string, we check if it exists
                    if (!val.is_string) return has_var(val.value);
                    // otherwise we return true of the string is not empty
                    return !val.value.empty();
                } else if constexpr (std::is_same_v<T, CondBinary>) {
                    // resolves a condexpr to a string
                    // so they could be compared later on
                    auto resolve = [&](const std::shared_ptr<CondExpr> &e) -> std::string {
                        if (!e) return "";
                        if (auto *lit = std::get_if<CondLiteral>(&e->value)) {
                            if (lit->is_string) return lit->value;
                            return resolve_var(lit->value);
                        }
                        return "";
                    };

                    switch (val.op) {
                        case CondOp::Eq:
                            return resolve(val.lhs) == resolve(val.rhs);
                        case CondOp::Neq:
                            return resolve(val.lhs) != resolve(val.rhs);
                        case CondOp::And:
                            return evaluate(val.lhs) && evaluate(val.rhs);
                        case CondOp::Or:
                            return evaluate(val.lhs) || evaluate(val.rhs);
                        default:
                            return false;
                    }
                } else if constexpr (std::is_same_v<T, CondUnary>) {
                    if (val.op == CondOp::Not) return !evaluate(val.operand);
                    return false;
                }

                return false;
            },
            expr->value);
    }

    ProcessResult Preprocessor::process(std::string_view source) const {
        ProcessResult result;

        DirectiveLexer lexer(source);
        auto           directives = lexer.scan();
        // no directives
        if (directives.empty()) {
            result.output = std::string(source);
            return result;
        }

        // splits the raw source into lines
        std::vector<std::string_view> lines;
        {
            size_t start = 0;
            for (size_t i = 0; i < source.size(); i++) {
                if (source[i] == '\n') {
                    lines.push_back(source.substr(start, i - start + 1));
                    start = i + 1;
                }
            }
            if (start < source.size()) {
                lines.push_back(source.substr(start));
            }
        }

        // indexing directives by line
        std::unordered_map<size_t, const Directive *> dir_by_line;
        for (const auto &d : directives) {
            dir_by_line[d.line] = &d;
        }

        struct IfState {
            bool any_branch_taken = false;
            bool current_active   = false;
        };

        // a stack of IfState is used to track nested #if, #elif, #else and #end blocks
        std::vector<IfState> stack;

        // checks if all levels of stack are active. meaning if we are inside
        // a true branch at every nesting level
        auto is_emmiting = [&]() -> bool {
            for (const auto &s : stack) {
                if (!s.current_active) return false;
            }
            return true;
        };

        // all levels are active except the current one. used by
        // #elif and #else to know if we should even evalute the current level
        auto parent_active = [&]() -> bool {
            for (size_t s = 0; s + 1 < stack.size(); s++) {
                if (!stack[s].current_active) return false;
            }
            return true;
        };

        std::ostringstream out;

        // for each line, if its a directive, we update the stack and emit a blank line
        // for example if
        //
        // #if arch == "x86_64" && os_family == "unix"
        // some code
        // #else
        // another code
        // #end
        //
        // then the #else up to #end and the directive lines themeselves
        // are replaced by blank lines. Blank lines preserve the line numbering
        // in case user needs line numbers for error reportation
        // Code that is not relevant to the manifest.toml config dont make it to the compiler
        for (size_t i = 0; i < lines.size(); i++) {
            size_t line_num = i + 1;

            auto it = dir_by_line.find(line_num);
            if (it != dir_by_line.end()) {
                const Directive &dir = *it->second;
                switch (dir.kind) {
                    case Zolal::DirectiveKind::If: {
                        bool cond = is_emmiting() && evaluate(dir.condition);
                        stack.push_back({cond, cond});
                        break;
                    }
                    case Zolal::DirectiveKind::Elif: {
                        if (stack.empty()) {
                            result.errors.push_back({line_num, "#elif without #if"});
                            break;
                        }
                        auto &top = stack.back();
                        if (top.any_branch_taken) {
                            top.current_active = false;
                        } else {
                            bool cond            = parent_active() && evaluate(dir.condition);
                            top.current_active   = cond;
                            top.any_branch_taken = top.any_branch_taken || cond;
                        }
                        break;
                    }
                    case Zolal::DirectiveKind::Else: {
                        if (stack.empty()) {
                            result.errors.push_back({line_num, "#else without #if"});
                            break;
                        }
                        auto &top            = stack.back();
                        top.current_active   = parent_active() && !top.any_branch_taken;
                        top.any_branch_taken = true;
                        break;
                    }
                    case Zolal::DirectiveKind::End: {
                        if (stack.empty()) {
                            result.errors.push_back({line_num, "#end without #if"});
                            break;
                        }
                        stack.pop_back();
                        break;
                    }
                }
                // directive lines become blank
                out << '\n';
                continue;
            }

            if (is_emmiting())
                out << lines[i];
            else
                out << '\n';
        }
        if (!stack.empty()) {
            result.errors.push_back({lines.size(), "Undetermined #if"});
        }

        result.output = out.str();
        return result;
    }

} // namespace Zolal
