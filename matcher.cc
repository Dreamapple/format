// Copyright (c) 2022, Tencent Inc.
//
// All rights reserved.
//
// Author: linghuimeng<linghuimeng@tencent.com>
// Date: 2022.03.14

#include "matcher.h"
namespace fp {


class FormatTokenizer {
 public:
  FormatTokenizer(std::string s) : s_(s), pos_(0), next_(), has_next_(true), mode_(kLiteralMode) { }

  void SetLiteralMode() {
    mode_ = kLiteralMode;
    printf("Switch mode=kLiteralMode\n");
  }
  void SetMatchMode() {
    mode_ = kMatchMode;
    printf("Switch mode=kMatchMode\n");
  }
  void SetLimitLiteralMode() {
    mode_ = kLimitLiteralMode;
    printf("Switch mode=kLimitLiteralMode\n");
  }

  MatchToken GetNext() {
    last_ = TryGetNext();
    printf("GetToken: Token(\"%s\", %d, %d)\n", last_.GetString().c_str(), last_.GetPos(), last_.GetType());
    return last_;
  }

  MatchToken TryGetNext() {
    if (mode_ == kLiteralMode || mode_ == kLimitLiteralMode) {
      return GetNextLiteralToken();
    }
    if (mode_ == kMatchMode) {
      return GetNextMatchToken();
    }
    return MatchToken();
  }

  MatchToken GetLast() {
    return last_;
  }
  bool HasNext() {
    if (pos_ >= s_.size()) {
      return false;
    }
    return has_next_ ;
  }

 private:

  // any char until
  MatchToken GetNextLiteralToken() {
    int pos = pos_;
    while (pos < s_.size()) {
      char ch = s_[pos];
      if (ch == '{' || ch == '}' || (mode_ == kLimitLiteralMode && (ch == '(' || ch == ')' || ch == ','))) {
        if (pos == pos_) {
          MatchToken ret(std::string(1, ch), pos, kFormatTokenTypeSymbol);
          pos_ += 1;
          // mode switch outer
          return ret;
        } else {
          MatchToken ret(s_.substr(pos_, pos-pos_), pos, kFormatTokenTypeLiteral);
          pos_ = pos;
          return ret;
        }
      } else if (ch == '\\') {
        pos += 2;
        continue;
      } else {
        pos += 1;
      }
    }
    if (has_next_ && pos > pos_) {
      MatchToken ret(s_.substr(pos_, pos-pos_), pos_, kFormatTokenTypeLiteral);
      has_next_ = false;
      pos_ = pos;
      return ret;
    }
    return MatchToken();
  }

  // {name:type}
  // {Decl(...)}
  // {name:type:Decl(...)}
  MatchToken GetNextMatchToken() {
    SkipWhite();

    static std::string id_token_s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_";
    static std::set<char> id_token{id_token_s.begin(), id_token_s.end()};
    int pos = pos_;
    while (pos < s_.size()) {
      char ch = s_[pos];
      if (id_token.count(ch) == 0) {
        if (pos == pos_) {
          MatchToken ret(std::string(1, ch), pos, kFormatTokenTypeSymbol);
          pos_ += 1;
          return ret;
        } else {
          MatchToken ret(s_.substr(pos_, pos-pos_), pos_, kFormatTokenTypeIdentifier);
          pos_ = pos;
          return ret;
        }
      } else if (ch == '\\') {
        pos += 2;
        continue;
      } else {
        pos += 1;
      }
    }
    if (has_next_ && pos > pos_) {
      MatchToken ret(s_.substr(pos_, pos-pos_), pos_, kFormatTokenTypeLiteral);
      has_next_ = false;
      pos_ = pos;
      return ret;
    }
    return MatchToken("", pos_, kFormatTokenTypeEOF);
  }

  void SkipWhite() {
    while (pos_ < s_.size()) {
      char ch = s_[pos_];
      if (ch == ' ' || ch == '\t' || ch == '\n') {
        pos_ += 1;
      } else {
        break;
      }
    }
  }

 private:
  enum TokenizerMode {
    kLiteralMode,
    kMatchMode,
    kLimitLiteralMode,
  };
  std::string s_;
  int pos_;
  std::string next_;
  bool has_next_;
  TokenizerMode mode_;
  MatchToken last_;
};


class FormatAstNode {
 public:
  FormatAstNode() = default;
  virtual ~FormatAstNode() = default;

  virtual void Dump(int d=0) {
    std::string tap(d, ' ');
    printf("%sFormatAstNode()\n", tap.c_str());
  }
};

class FormatRootNode : public FormatAstNode {
 public:
  void Append(std::shared_ptr<FormatAstNode> node) {
    elements_.push_back(node);
  }

  int GetElementsSize() {
    return (int) elements_.size();
  }

  virtual void Dump(int d=0) {
    std::string tap(d, ' ');
    printf("%sFormatRootNode() {\n", tap.c_str());

    for (auto item: elements_) {
      item->Dump(d+2);
    }
    printf("%s}\n", tap.c_str());
  }
 private:
  std::vector<std::shared_ptr<FormatAstNode>> elements_;
};

class FormatLiteralNode: public FormatAstNode {
 public:
  FormatLiteralNode(MatchToken token) : token_(token) {}

  virtual void Dump(int d=0) {
    std::string tap(d, ' ');
    printf("%sFormatRootNode(MatchToken(%s, %d, %d))\n",
           tap.c_str(), token_.GetString().c_str(), token_.GetPos(), token_.GetType());
  }
 private:
  MatchToken token_;
};

class FormatDeclNode: public FormatAstNode {
 public:
  void SetName(MatchToken token) { name_ = token; }
  bool HasName() { return !name_.IsEmpty(); }
  void AppendParam(std::shared_ptr<FormatRootNode> node) { elements_.push_back(node); }

  virtual void Dump(int d=0) {
    std::string tap(d, ' ');
    printf("%sFormatDeclNode(name=MatchToken(%s, %d, %d)) {\n",
           tap.c_str(), name_.GetString().c_str(), name_.GetPos(), name_.GetType());
    for (auto item: elements_) {
      item->Dump(d+2);
    }
    printf("%s}\n", tap.c_str());
  }

 private:
  MatchToken name_;
  std::vector<std::shared_ptr<FormatRootNode>> elements_;
};

class FormatMatcherNode: public FormatAstNode {
 public:
  FormatMatcherNode() = default;
  void SetName(MatchToken token) { name_ = token; }
  void SetType(MatchToken token) { type_ = token; }
  void SetSpec(MatchToken token) { spec_ = token; }
  void SetDecl(std::shared_ptr<FormatDeclNode> node) { decl_ = node; }

  bool HasName() { return !name_.IsEmpty(); }
  bool HasType() { return !type_.IsEmpty(); }
  bool HasSpec() { return !spec_.IsEmpty(); }
  bool HasDecl() { return !decl_; }

  virtual void Dump(int d=0) {
    std::string tap(d, ' ');
    printf("%sFormatMatcherNode(name=%s, type=%s, spec=%s) {\n",
           tap.c_str(), name_.GetString().c_str(), type_.GetString().c_str(), spec_.GetString().c_str());
    if (decl_) {
      decl_->Dump(d + 2);
    }
    printf("%s}\n", tap.c_str());
  }

 private:
  MatchToken name_, type_, spec_;
  std::shared_ptr<FormatDeclNode> decl_;
};

class FormatParser {
 public:
  FormatParser() = default;

  int Parse(const std::string& str, FormatRootNode& root) {

    FormatTokenizer tokenizer(str);
    return ParseElements(tokenizer, root);
  }

  int ParseElements(FormatTokenizer& tokenizer, FormatRootNode& root) {
    while (tokenizer.HasNext()) {
      auto token = tokenizer.GetNext();

      if (token.GetType() == kFormatTokenTypeLiteral) {
        std::shared_ptr<FormatAstNode> node(new FormatLiteralNode(token));
        root.Append(node);
      } else if (token.GetType() == kFormatTokenTypeEOF) {
        return 0;
      } else if (token.GetType() == kFormatTokenTypeIdentifier) {
        // 不应该走到这里 报错
        return -1;
      } else if (token.GetString() == "{") {
        std::shared_ptr<FormatMatcherNode> node(new FormatMatcherNode());
        root.Append(node);
        int ret = ParseMatch(tokenizer, node);
        return ret;
      } else {
        // 不应该走到这里
        return -2;
      }
    } // end while
    return 0;
  }

  int ParseMatch(FormatTokenizer& tokenizer, std::shared_ptr<FormatMatcherNode> matcher) {
    tokenizer.SetMatchMode();
    while (tokenizer.HasNext()) {
      auto first_token  = tokenizer.GetNext();  // 先拿出来一个，需要看下一个token决定类型
      auto second_token = tokenizer.GetNext();

      if (second_token.GetString() == ":" || second_token.GetString() == "}") {
        if (!matcher->HasName()) {
          matcher->SetName(first_token);
        } else if (!matcher->HasType() && first_token.GetString().size() > 2) {
          matcher->SetType(first_token);
        } else if (!matcher->HasSpec() && first_token.GetString().size() == 2) {
          matcher->SetSpec(first_token);
        } else {
          // err
          return -3;
        }
      } else if (second_token.GetString() == "(") {
        std::shared_ptr<FormatDeclNode> node(new FormatDeclNode());
        node->SetName(first_token);
        matcher->SetDecl(node);
        int ret = ParseParamsList(tokenizer, node);
        tokenizer.SetLiteralMode();
        return ret;
      } else {

      }
    }
    return -4;
  }

  int ParseParamsList(FormatTokenizer& tokenizer, std::shared_ptr<FormatDeclNode> decl) {
    tokenizer.SetLimitLiteralMode(); // 对字符 {} () , 敏感
    while (tokenizer.HasNext()) {
      std::shared_ptr<FormatRootNode> node(new FormatRootNode);
      int ret = ParseDeclParam(tokenizer, node);
      if (ret != 0) {
        return ret;
      }
      decl->AppendParam(node);
      auto last_token = tokenizer.GetLast();
      if (last_token.GetString() == ")") {
        return 0;
      }
    }
    return -5;
  }

  int ParseDeclParam(FormatTokenizer& tokenizer, std::shared_ptr<FormatRootNode> root) {
    bool ignore_comma = false;
    while (tokenizer.HasNext()) {
      auto token = tokenizer.GetNext();

      if (token.GetType() == kFormatTokenTypeLiteral) {
        std::shared_ptr<FormatAstNode> node(new FormatLiteralNode(token));
        root->Append(node);
      } else if (token.GetType() == kFormatTokenTypeEOF) {
        return -6;
      } else if (token.GetType() == kFormatTokenTypeIdentifier) {
        // 不应该走到这里 报错
        return -7;
      } else if (token.GetString() == "{") {
        std::shared_ptr<FormatMatcherNode> node(new FormatMatcherNode());
        root->Append(node);
        int ret = ParseMatch(tokenizer, node);
        if (ret != 0) {
          return ret;
        }
      } else if (token.GetString() == "(") {
        if (root->GetElementsSize() == 0) {
          ignore_comma = true;
        } else {
          std::shared_ptr<FormatLiteralNode> node(new FormatLiteralNode(token));
          root->Append(node);
        }
      } else if (token.GetString() == ")") {
        if (ignore_comma) {
          ignore_comma = false;
        } else {
          return 0;
        }
      } else if (token.GetString() == ",") {
        if (ignore_comma) {
          std::shared_ptr<FormatLiteralNode> node(new FormatLiteralNode(token));
          root->Append(node);
        } else {
          return 0;
        }
      }
      else {
        // 不应该走到这里
        return -2;
      }
    } // end while
    return 0;
  }
};

}