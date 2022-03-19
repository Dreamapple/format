// Copyright (c) 2022, Tencent Inc.
//
// All rights reserved.
//
// Author: linghuimeng<linghuimeng@tencent.com>
// Date: 2022.03.14

#pragma once
#include <string>
#include <vector>
#include <stack>
#include <set>
#include <map>
#include <memory>
#include "tokenizer.h"

namespace fq {
class ResultItem {
 public:
  ResultItem() = default;
  ResultItem(std::string name, std::string type, std::string value)
      :name_(name), type_(type), value_(value) {}

// private:
  std::string name_, type_, value_;
};

class MatchResult {
 public:
  bool Set(std::string name, std::string type, std::string value) {
    results_[name] = ResultItem(name, type, value);
    return true;
  }

  void Dump() {
    for (auto it: results_) {
      printf("ResultItem '%s' '%s' '%s'\n", it.second.name_.c_str(), it.second.type_.c_str(), it.second.value_.c_str() );
    }
  }

 private:
  std::map<std::string, ResultItem> results_;
};

class FormatAstNode {
 public:
  FormatAstNode() = default;
  virtual ~FormatAstNode() = default;

  virtual void Dump(int d=0) {
    std::string tap(d, ' ');
    printf("%sFormatAstNode()\n", tap.c_str());
  }

  virtual bool IsLiteral() {
    return false;
  }
  virtual bool Search(std::string s, int start, int& match_start, int& match_stop) {
    return false;
  }
  virtual bool Handle(std::string s, int start, int stop, MatchResult& result) {
    return false;
  }
};

class FormatRootNode : public FormatAstNode {
 public:
  bool Handle(std::string s, int start, int stop, MatchResult& result) override {
    std::shared_ptr<FormatAstNode> pending;
    for (auto element: elements_) {
      if (element->IsLiteral()) {
        int match_start, match_stop;
        bool found = element->Search(s, start, match_start, match_stop);
        if (!found) {
          return false;
        }
        if (match_stop > stop) {
          return false;
        }
        if (pending) {
          if (!pending->Handle(s, start, match_start, result)) {
            return false;
          } else {
            pending.reset();
          }
        } else {
          if (match_start != 0) {
            return false;
          }
        }
        start = match_stop;
      } else {
        if (pending) {
          return false;
        } else {
          pending = element;
        }
      }
    }
    if (pending) {
      return pending->Handle(s, start, stop, result);
    }
    return true;
  }
  void Append(std::shared_ptr<FormatAstNode> node) {
    elements_.push_back(node);
  }

  int GetElementsSize() {
    return (int) elements_.size();
  }

  virtual void Dump(int d=0) override {
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
  FormatLiteralNode(Token token) : token_(token) {}

  bool IsLiteral() override { return true; }
  virtual void Dump(int d=0) override {
    std::string tap(d, ' ');
    printf("%sFormatLiteralNode(Token(%s, %d, %d))\n",
           tap.c_str(), token_.GetString().c_str(), token_.GetPos(), token_.GetType());
  }
  bool Search(std::string s, int start, int& match_start, int& match_stop) override {
    auto pos = s.find(token_.GetString(), start);
    if (pos == std::string::npos) {
      return false;
    }
    match_start = pos;
    match_stop = pos + token_.GetString().size();
    return true;
  }
 private:
  Token token_;
};

class FormatDeclNode: public FormatAstNode {
 public:
  void SetName(Token token) { name_ = token; }
  bool HasName() { return !name_.IsEmpty(); }
  void AppendParam(std::shared_ptr<FormatRootNode> node) { elements_.push_back(node); }

  virtual void Dump(int d=0) override {
    std::string tap(d, ' ');
    printf("%sFormatDeclNode(name=Token(%s, %d, %d)) {\n",
           tap.c_str(), name_.GetString().c_str(), name_.GetPos(), name_.GetType());
    for (auto item: elements_) {
      item->Dump(d+2);
    }
    printf("%s}\n", tap.c_str());
  }

  bool Handle(std::string s, int start, int stop, MatchResult& result) override {
    if (!HasName()) {
      return false;
    }

    auto func_name = name_.GetString();
    if (func_name == "Raw") {
      if (elements_.size() != 1) {
        return false;
      }
      auto value = s.substr(start, stop-start);
      return elements_.at(0)->Handle(value, 0, value.size(), result);
    }
/*
    if (func_name == "Base64") {
      if (elements_.size() != 1) {
        return false;
      }
      auto value = DecodeBase64(s.substr(start, stop-start));
      return elements_.at(0)->Handle(value, 0, value.size(), result);
    }
    if (func_name == "FKV") {
      if (elements_.size() != 3) {
        return false;
      }
      auto value = s.substr(start, stop-start);
      FKVHandler
    }*/
    return false;
  }


 private:
  Token name_;
  std::vector<std::shared_ptr<FormatRootNode>> elements_;
};

class FormatMatcherNode: public FormatAstNode {
 public:
  FormatMatcherNode() = default;
  void SetName(Token token) { name_ = token; }
  void SetType(Token token) { type_ = token; }
  void SetSpec(Token token) { spec_ = token; }
  void SetDecl(std::shared_ptr<FormatDeclNode> node) { decl_ = node; }

  bool HasName() { return !name_.IsEmpty(); }
  bool HasType() { return !type_.IsEmpty(); }
  bool HasSpec() { return !spec_.IsEmpty(); }
  bool HasDecl() { return bool(decl_); }

  virtual void Dump(int d=0) override {
    std::string tap(d, ' ');
    printf("%sFormatMatcherNode(name=%s, type=%s, spec=%s) {\n",
           tap.c_str(), name_.GetString().c_str(), type_.GetString().c_str(), spec_.GetString().c_str());
    if (decl_) {
      decl_->Dump(d + 2);
    }
    printf("%s}\n", tap.c_str());
  }

  bool Handle(std::string s, int start, int stop, MatchResult& result) override {
    if (!HasDecl()) {
      if (!HasName()) {
        return false;
      }
      return result.Set(name_.GetString(), type_.GetString(), s.substr(start, stop-start));
    } else {
      return decl_->Handle(s, start, stop, result);
    }

  }

 private:
  Token name_, type_, spec_;
  std::shared_ptr<FormatDeclNode> decl_;
};

class FormatParser {
 public:
  FormatParser(bool debug=false) : debug_(debug) {}

  int Parse(const std::string& str, FormatRootNode& root) {

    Tokenizer tokenizer(str, debug_);
    return ParseElements(tokenizer, root);
  }

  int ParseElements(Tokenizer& tokenizer, FormatRootNode& root) {
    while (tokenizer.HasNext()) {
      auto token = tokenizer.GetNext();

      if (token.GetType() == kTokenTypeLiteral) {
        std::shared_ptr<FormatAstNode> node(new FormatLiteralNode(token));
        root.Append(node);
      } else if (token.GetType() == kTokenTypeEOF) {
        return 0;
      } else if (token.GetType() == kTokenTypeIdentifier) {
        // 不应该走到这里 报错
        return -1;
      } else if (token.GetString() == "{") {
        std::shared_ptr<FormatMatcherNode> node(new FormatMatcherNode());
        root.Append(node);
        int ret = ParseMatch(tokenizer, node);
        if (ret != 0) {
          return ret;
        }
      } else {
        // 不应该走到这里
        return -2;
      }
    } // end while
    return 0;
  }

  int ParseMatch(Tokenizer& tokenizer, std::shared_ptr<FormatMatcherNode> matcher) {
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
        if (second_token.GetString() == "}") {
          return 0;
        }
      } else if (second_token.GetString() == "(") {
        std::shared_ptr<FormatDeclNode> node(new FormatDeclNode());
        node->SetName(first_token);
        matcher->SetDecl(node);
        int ret = ParseParamsList(tokenizer, node);
        if (ret != 0) {
          return ret;
        }
        auto end = tokenizer.GetNext();
        if (end.GetString() != "}") {
          return -9;
        }
        return 0;
      } else {

      }
    }
    return -4;
  }

  int ParseParamsList(Tokenizer& tokenizer, std::shared_ptr<FormatDeclNode> decl) {
    while (tokenizer.HasNext()) {
      std::shared_ptr<FormatRootNode> node(new FormatRootNode());
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

  int ParseDeclParam(Tokenizer& tokenizer, std::shared_ptr<FormatRootNode> root) {
    bool ignore_comma = false;
    while (tokenizer.HasNext()) {
      auto token = tokenizer.GetNext();

      if (token.GetType() == kTokenTypeLiteral) {
        std::shared_ptr<FormatAstNode> node(new FormatLiteralNode(token));
        root->Append(node);
      } else if (token.GetType() == kTokenTypeEOF) {
        return -6;
      } else if (token.GetType() == kTokenTypeIdentifier) {
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
        return -10;
      }
    } // end while
    return 0;
  }

 private:
  bool debug_ = false;
};

}