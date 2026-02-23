#include <cctype>
#include <memory>
#include <zolal/DirectiveLexer.hpp>

namespace Zolal {

    char DirectiveLexer::peek() const {
        if (at_end()) return '\0';
        return source[pos];
    }

    char DirectiveLexer::advance() {
        char c = source[pos++];
        if (c == '\n') line++;
        return c;
    }

    bool DirectiveLexer::at_end() const {
        return pos >= source.size();
    }

    void DirectiveLexer::skip_to_eol() {
        while (!at_end() && peek() != '\n') advance();
    }

    void DirectiveLexer::skip_string() {
        char quote = advance();
        while (!at_end()) {
            char c = advance();
            if (c == '\\' && !at_end()) {
                advance();
            } else if (c == quote) {
                return;
            }
        }
    }

    void DirectiveLexer::skip_block_comment() {
        // skipping the '/*'
        advance();
        advance();

        while (!at_end()) {
            if (peek() == '*' && pos + 1 < source.size() && source[pos + 1] == '/') {
                advance();
                advance();
                return;
            }
            advance();
        }
    }

    void DirectiveLexer::skip_line_comment() {
        advance();
        advance();
        skip_to_eol();
    }

    bool DirectiveLexer::at_line_start() const {
        if (pos == 0) return true;
        // walk backwards frm current pos to check only white spaces before
        size_t p = pos - 1;
        while (p > 0 && source[p] != '\n' && std::isspace(source[p])) {
            p--;
        }
        return p == 0 || source[p] == '\n';
    }

    std::vector<Directive> DirectiveLexer::scan() {
        std::vector<Directive> directives;

        while (!at_end()) {
            // skip strs
            if (peek() == '"') {
                skip_string();
                continue;
            }

            if (peek() == '/' && pos + 1 < source.size()) {
                if (source[pos + 1] == '/') {
                    skip_line_comment();
                    continue;
                }
                if (source[pos + 1] == '*') {
                    skip_block_comment();
                    continue;
                }
            }

            // check for # at the start of line
            // also allows leading white space
            if (peek() == '#' && at_line_start()) {
                Directive dir;
                dir.start_pos = pos;
                dir.line      = line;
                // skipping #
                advance();

                std::string keyword;
                while (!at_end() && std::isalpha(peek())) {
                    keyword += advance();
                }

                DirectiveKind kind;
                if (keyword == "if")
                    kind = DirectiveKind::If;
                else if (keyword == "elif")
                    kind = DirectiveKind::Elif;
                else if (keyword == "else")
                    kind = DirectiveKind::Else;
                else if (keyword == "end")
                    kind = DirectiveKind::End;
                else {
                    // unknown! skipping
                    skip_to_eol();
                    continue;
                }

                dir.kind = kind;

                // for #if or #elif we read the rest of the line as condition
                if (kind == DirectiveKind::If || kind == DirectiveKind::Elif) {
                    // skipping whitespace
                    while (!at_end() && peek() != '\n' && std::isspace(peek())) {
                        advance();
                    }

                    std::string cond_text;
                    while (!at_end() && peek() != '\n') {
                        cond_text += advance();
                    }
                    dir.condition = parse_condition(cond_text);
                } else {
                    skip_to_eol();
                }

                dir.end_pos = pos;
                directives.push_back(std::move(dir));
                continue;
            }
            advance();
        }
        return directives;
    }

    void DirectiveLexer::skip_ws(const std::string &text, size_t &p) {
        while (!at_end() && std::isspace(text[p])) {
            p++;
        }
    }

    std::string DirectiveLexer::parse_string_lit(const std::string &text, size_t &p) {
        // skipping "
        p++;
        std::string result;
        while (p < text.size() && text[p] != '"') {
            if (text[p] == '\\' && p + 1 < text.size()) {
                p++;
                result += text[p];
            } else {
                result += text[p];
            }
            p++;
        }
        // skipping "
        if (p < text.size()) {
            p++;
        }
        return result;
    }

    std::string DirectiveLexer::parse_identifier(const std::string &text, size_t &p) {
        std::string result;

        while (p < text.size() && (std::isalnum(text[p]) || text[p] == '_')) {
            result += text[p++];
        }
        return result;
    }

    std::shared_ptr<CondExpr> DirectiveLexer::parse_condition(const std::string &text) {
        size_t p = 0;
        return parse_or(text, p);
    }

    // or_expr = and_expr ( "||" and_expr)*
    std::shared_ptr<CondExpr> DirectiveLexer::parse_or(const std::string &text, size_t &p) {
        auto left = parse_and(text, p);
        skip_ws(text, p);
        while (p + 1 < text.size() && text[p] == '|' && text[p + 1] == '|') {
            p += 2;
            auto right  = parse_and(text, p);
            auto node   = std::make_shared<CondExpr>();
            node->value = CondBinary{CondOp::Or, left, right};
            left        = node;
            skip_ws(text, p);
        }
        return left;
    }

    // and_expr = unary_expr ( "&&" unary_expr)*
    std::shared_ptr<CondExpr> DirectiveLexer::parse_and(const std::string &text, size_t &p) {
        auto left = parse_unary(text, p);
        skip_ws(text, p);
        while (p + 1 < text.size() && text[p] == '&' && text[p + 1] == '&') {
            p += 2;
            auto right  = parse_unary(text, p);
            auto node   = std::make_shared<CondExpr>();
            node->value = CondBinary{CondOp::And, left, right};
            left        = node;
            skip_ws(text, p);
        }
        return left;
    }

    /// unary_expr = "!" unary_expr | comparison
    std::shared_ptr<CondExpr> DirectiveLexer::parse_unary(const std::string &text, size_t &p) {
        skip_ws(text, p);
        if (p < text.size() && text[p] == '!') {
            p++;
            auto operand = parse_unary(text, p);
            auto node    = std::make_shared<CondExpr>();
            node->value  = CondUnary{CondOp::Not, operand};
            return node;
        }
        return parse_comparison(text, p);
    }

    // comparison = primary ( ("==" | "!=" ) primary)?
    std::shared_ptr<CondExpr> DirectiveLexer::parse_comparison(const std::string &text, size_t &p) {
        auto left = parse_primary(text, p);

        skip_ws(text, p);

        if (p + 1 < text.size()) {
            CondOp op;
            bool   found = false;

            if (text[p] == '=' && text[p + 1] == '=') {
                op    = CondOp::Eq;
                found = true;
            } else if (text[p] == '!' && text[p + 1] == '=') {
                op    = CondOp::Neq;
                found = true;
            }
            if (found) {
                p += 2;
                auto right  = parse_primary(text, p);
                auto node   = std::make_shared<CondExpr>();
                node->value = CondBinary{op, left, right};
                return node;
            }
        }
        return left;
    }

    // primary = "(" or_expr ")" | string_literal | identifier
    std::shared_ptr<CondExpr> DirectiveLexer::parse_primary(const std::string &text, size_t &p) {
        skip_ws(text, p);
        auto node = std::make_shared<CondExpr>();

        if (p < text.size() && text[p] == '(') {
            p++;
            auto inner = parse_or(text, p);
            skip_ws(text, p);
            if (p < text.size() && text[p] == ')') p++;
            return inner;
        }

        if (p < text.size() && text[p] == '"') {
            node->value = CondLiteral{parse_string_lit(text, p), true};
            return node;
        }

        node->value = CondLiteral{parse_identifier(text, p), false};
        return node;
    }

} // namespace Zolal
