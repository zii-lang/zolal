#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace Zolal {

    enum class DirectiveKind {
        If,
        Elif,
        Else,
        End,
    };

    /// a condition expression node
    /// supports: identifiers = "string", identifier != "string",
    ///           expr && expr, expr || expr , !expr, (expr)
    struct CondExpr;

    enum class CondOp { Eq, Neq, And, Or, Not };

    struct CondLiteral {
        /// a config variable name or string literal
        std::string value;
        bool        is_string = false;
    };

    struct CondBinary {
        CondOp                    op;
        std::shared_ptr<CondExpr> lhs;
        std::shared_ptr<CondExpr> rhs;
    };

    struct CondUnary {
        CondOp                    op;
        std::shared_ptr<CondExpr> operand;
    };

    struct CondExpr {
        std::variant<CondLiteral, CondBinary, CondUnary> value;
    };

    /// a directive found in source
    struct Directive {
        DirectiveKind kind;
        // 1-indexed
        size_t line;
        // byte offset in source
        size_t start_pos;
        // byte offset past the directive line
        size_t end_pos;
        // null for #else / #end
        std::shared_ptr<CondExpr> condition;
    };

    /// represents a span of source that is either passthrough or a directive
    struct SourceSpan {
        /// byte offsets
        size_t start;
        size_t end;
        /// starting line (1-indexed)
        size_t line;
    };

    class DirectiveLexer {
      private:
        std::string_view source;
        size_t           pos  = 0;
        size_t           line = 1;

        char peek() const;
        char advance();
        bool at_end() const;
        void skip_to_eol();
        void skip_string();
        void skip_line_comment();
        void skip_block_comment();
        bool at_line_start() const;

        std::shared_ptr<CondExpr> parse_condition(const std::string &text);
        std::shared_ptr<CondExpr> parse_or(const std::string &text, size_t &p);
        std::shared_ptr<CondExpr> parse_and(const std::string &text, size_t &p);
        std::shared_ptr<CondExpr> parse_unary(const std::string &text, size_t &p);
        std::shared_ptr<CondExpr> parse_primary(const std::string &text, size_t &p);
        std::shared_ptr<CondExpr> parse_comparison(const std::string &text, size_t &p);

        void        skip_ws(const std::string &text, size_t &p);
        std::string parse_string_lit(const std::string &text, size_t &p);
        std::string parse_identifier(const std::string &text, size_t &p);

      public:
        explicit DirectiveLexer(std::string_view src)
            : source(src) {}

        /// scans the entire source code and returns all directives found
        std::vector<Directive> scan();
    };

} // namespace Zolal
