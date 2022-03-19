// Copyright (c) 2022, Tencent Inc.
//
// All rights reserved.
//
// Author: linghuimeng<linghuimeng@tencent.com>
// Date: 2022.03.14

#pragma once
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <vector>

namespace fq {

// token 类型
enum TokenType {
  kTokenTypeLiteral    = 1,  // 文本类型
  kTokenTypeIdentifier = 2,  // ID
  kTokenTypeSymbol     = 3,  // 符号
  kTokenTypeEOF        = 4,  // 空
};

class Token {
 public:
  Token() {}
  Token(std::string s, int pos, TokenType type) : empty(false), token_(s), pos_(pos), type_(type) {}

  std::string GetString() { return token_; }
  TokenType GetType() { return type_; }
  int GetPos() { return pos_; }

  bool IsEmpty() { return empty; }

  std::string Repr() { return ""; }

  bool operator==(const Token& other) { return token_ == other.token_ && pos_ == other.pos_ && type_ == other.type_; }

 private:
  bool empty = true;
  std::string token_;
  int pos_;
  TokenType type_;
};

class Tokenizer {
 public:
  Tokenizer(const std::string& pattern, bool debug = false);

  Token GetNext() {
    TokenizerMode before = inner_mode_.top();
    last_                = TryGetNext();
    TokenizerMode after  = inner_mode_.top();
    if (debug_) {
      printf("[#%d->%d] GetToken: Token(\"%s\", %d, %d)\n",
             before,
             after,
             last_.GetString().c_str(),
             last_.GetPos(),
             last_.GetType());
    }
    return last_;
  }

  Token GetLast() { return last_; }
  bool HasNext() { return pos_ < pattern_.size(); }

 private:
  Token TryGetNext();

  // any char until '{'
  Token GetNextLiteralToken();

  // {name:type}
  // {Decl(...)}
  // {name:type:Decl(...)}
  Token GetNextMatchToken();

  // (... , ...)
  Token GetNextTokenInParamsList();

  //
  void SkipWhite();

 private:
  enum TokenizerMode {
    // 最外层的文本匹配状态 该状态只对'{'敏感，一旦匹配到'{' 状态切换为 kMatchMode
    kLiteralMode,

    // {...} 中数据的状态
    // 该状态 ID为 [a-zA-Z0-9_]的匹配
    // 各个字符都有其含义
    // 各个字段间使用':'分割
    //
    // 遇到'}'转换为上一个状态
    // 遇到 '(' 进入 kMatchParamsMode
    kMatchMode,

    // (..., ..) 参数状态
    // 遇到')'返回kMatchMode
    // 遇到其它非空字符进入kLimitLiteralMode
    kMatchParamsMode,

    // 类似kLiteralMode但是是在内部的文本匹配
    // 如果第一个字符是'(' 那么遇到')' + ',' 返回上一个状态
    // 否则只需要遇到','返回上一个状态
    kLimitLiteralModeBrace,  // meet '('
    kLimitLiteralMode,       // meet other char
  };

  // 需要切分的原始字符串
  std::string pattern_;
  // 当前处理位置
  int pos_ = 0;

  // 切分模式
  std::stack<TokenizerMode> inner_mode_;

  // 上一个token
  Token last_;

  bool debug_ = false;
};

}  // namespace fq
