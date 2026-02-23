#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <zolal/DirectiveLexer.hpp>
#include <zolal/Manifest.hpp>

using namespace Z::Manifest;
namespace Zolal {
    struct ProcessError {
        size_t      line;
        std::string message;
    };

    struct ProcessResult {
        std::string               output;
        std::vector<ProcessError> errors;
        bool                      ok() const {
            return errors.empty();
        }
    };

    class Preprocessor {
      private:
        const Manifest &manifest;
        // derived from triplets: arch, vendor, os, env, pointer_width, endian, etc
        std::unordered_map<std::string, std::string> derived;

        void parse_triplets();
        bool evaluate(const std::shared_ptr<CondExpr> &expr) const;
        // resolves a variable name to its string value
        std::string resolve_var(const std::string &name) const;
        // checks if a variable name is defined
        bool has_var(const std::string &name) const;

      public:
        explicit Preprocessor(const Manifest &m)
            : manifest(m) {}

        ProcessResult process(std::string_view source) const;
    };

} // namespace Zolal
