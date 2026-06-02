require 'rouge'

module Rouge
  module Lexers
    class Z < RegexLexer
      title "Z"
      desc "The Z programming language"
      tag 'z'
      filenames '*.z'

      state :root do
        rule %r/\s+/, Text
        rule %r((//.*?$)), Comment::Single
        rule %r(=begin\b.*?\n=end\b)m, Comment::Multiline
        rule %r/\b(func|let|\#if|\#elif|\#else|\#end|type|alias|while|return|import|struct|enum|match|true|false|nil|alloc|check|endcheck|loop|endloop|reduce|lamb|vari|view|job|yield|fiber|serial|parallel|stop|lock|unlock|swap|flush|shared|break|continue|goto)\b/, Keyword
        rule %r/[{}()\[\];,]/, Punctuation
        rule %r/[_>]|[_<]|[:=]/, Operator
        rule %r/[-+*\/%=<>!&|^~]/, Operator
        rule %r/\d+/, Num::Integer
        rule %r/"([^"\\]|\\.)*"/, Str::Double
        rule %r/[a-zA-Z_]\w*/, Name
      end
    end
  end
end
