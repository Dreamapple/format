// Copyright (c) 2022, Tencent Inc.
//
// All rights reserved.
//
// Author: linghuimeng<linghuimeng@tencent.com>
// Date: 2022.03.14

#include "tokenizer.h"
namespace fq {

Tokenizer::Tokenizer(const std::string& pattern, bool debug) : pattern_(pattern), debug_(debug) {
  inner_mode_.push(kLiteralMode);
}

Token Tokenizer::TryGetNext() {
  TokenizerMode mode = inner_mode_.top();
  if (mode == kLiteralMode || mode == kLimitLiteralModeBrace || mode == kLimitLiteralMode) {
    return GetNextLiteralToken();
  }
  if (mode == kMatchMode) {
    return GetNextMatchToken();
  }
  if (mode == kMatchParamsMode) {
    return GetNextTokenInParamsList();
  }
  return Token();  // empty
}

// any char until
Token Tokenizer::GetNextLiteralToken() {
  TokenizerMode mode = inner_mode_.top();
  int pos            = pos_;
  while (pos < pattern_.size()) {
    char ch = pattern_[pos];
    if (ch == '{' || ch == '}' ||
        ((mode == kLimitLiteralModeBrace or mode == kLimitLiteralMode) && (ch == '(' || ch == ')' || ch == ','))) {
      if (pos == pos_) {
        Token ret(std::string(1, ch), pos_, kTokenTypeSymbol);
        pos_ += 1;
        // mode switch outer
        if (ch == '{') {
          inner_mode_.push(kMatchMode);
        }
        if (mode == kLimitLiteralModeBrace && ch == ')') {
          // mode = kLimitLiteralMode;
          inner_mode_.pop();
          // assert inner_mode_.top() == kLimitLiteralMode
        }
        if (mode == kLimitLiteralMode && ch == ',') {
          inner_mode_.pop();  // mode = kMatchParamsMode;
        }
        if (mode == kLimitLiteralMode && ch == ')') {
          // mode = kMatchMode;
          inner_mode_.pop();  // to kMatchParamsMode
          inner_mode_.pop();  // to kMatchMode
        }

        return ret;
      } else {
        Token ret(pattern_.substr(pos_, pos - pos_), pos_, kTokenTypeLiteral);
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
  if (pos > pos_) {
    Token ret(pattern_.substr(pos_, pos - pos_), pos_, kTokenTypeLiteral);
    pos_ = pos;
    return ret;
  }
  return Token("", pos_, kTokenTypeEOF);
}

// {name:type}
// {Decl(...)}
// {name:type:Decl(...)}
Token Tokenizer::GetNextMatchToken() {
  TokenizerMode mode = inner_mode_.top();
  SkipWhite();

  static std::string id_token_s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_<>+-";
  static std::set<char> id_token{id_token_s.begin(), id_token_s.end()};
  int pos = pos_;
  while (pos < pattern_.size()) {
    char ch = pattern_[pos];
    if (id_token.count(ch) == 0) {
      if (pos == pos_) {
        Token ret(std::string(1, ch), pos_, kTokenTypeSymbol);
        pos_ += 1;
        // tryGetNextState
        if (mode == kMatchMode && ch == '(') {
          // mode = kMatchParamsMode;
          inner_mode_.push(kMatchParamsMode);
        }
        if (mode == kMatchMode && ch == '}') {
          inner_mode_.pop();
          // mode = kLiteralMode; // 只能pop 要不如不知道哪里进来的
        }
        return ret;
      } else {
        Token ret(pattern_.substr(pos_, pos - pos_), pos_, kTokenTypeIdentifier);
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
  return Token("", pos_, kTokenTypeEOF);
}

Token Tokenizer::GetNextTokenInParamsList() {
  SkipWhite();
  inner_mode_.push(kLimitLiteralMode);
  char start_char = pattern_[pos_];
  if (start_char == '(') {
    inner_mode_.push(kLimitLiteralModeBrace);
  }
  return GetNextLiteralToken();
}

void Tokenizer::SkipWhite() {
  while (pos_ < pattern_.size()) {
    char ch = pattern_[pos_];
    if (ch == ' ' || ch == '\t' || ch == '\n') {
      pos_ += 1;
    } else {
      break;
    }
  }
}

}  // namespace fq