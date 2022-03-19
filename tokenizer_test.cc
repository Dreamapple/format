// Copyright (c) 2022, Tencent Inc.
//
// All rights reserved.
//
// Author: linghuimeng<linghuimeng@tencent.com>
// Date: 2022.03.15

#include "tokenizer.h"
#include <gtest/gtest.h>

using namespace fq;

TEST(Tokenizer, HandleSimpleLiteral)
{
  Tokenizer tokenizer("abc", true);
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("abc", 0, kTokenTypeLiteral));
  EXPECT_EQ(tokenizer.HasNext(), false);
}
TEST(Tokenizer, HandleSimpleLiteral2)
{
  Tokenizer tokenizer("abc\n\t\r\v()[];,.", true);
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("abc\n\t\r\v()[];,.", 0, kTokenTypeLiteral));
  EXPECT_EQ(tokenizer.HasNext(), false);
}
TEST(Tokenizer, HandleSimpleLiteral3)
{
  Tokenizer tokenizer("", true);
  EXPECT_EQ(tokenizer.HasNext(), false);
  EXPECT_EQ(tokenizer.GetNext(), Token("", 0, kTokenTypeEOF));
  EXPECT_EQ(tokenizer.HasNext(), false);
}
TEST(Tokenizer, HandleSimpleMatch)
{
  Tokenizer tokenizer("{name}", true);
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("{", 0, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("name", 1, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("}", 5, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.HasNext(), false);
}
TEST(Tokenizer, HandleSimpleMatch2)
{
  Tokenizer tokenizer("{name:type}", true);
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("{", 0, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("name", 1, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token(":", 5, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("type", 6, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("}", 10, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.HasNext(), false);
}
TEST(Tokenizer, HandleRecurentMatch)
{
  Tokenizer tokenizer("{name:type:FKV()}", true);
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("{", 0, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("name", 1, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.GetNext(), Token(":", 5, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("type", 6, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.GetNext(), Token(":", 10, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("FKV", 11, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.GetNext(), Token("(", 14, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token(")", 15, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("}", 16, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.HasNext(), false);
}
TEST(Tokenizer, HandleRecurentMatch2)
{
  Tokenizer tokenizer("{FKV()}", true);
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("{", 0, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("FKV", 1, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.GetNext(), Token("(", 4, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token(")", 5, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("}", 6, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.HasNext(), false);
}

TEST(Tokenizer, HandleRecurentMatch3)
{
  Tokenizer tokenizer("{FKV(102)}", true);
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("{", 0, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("FKV", 1, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.GetNext(), Token("(", 4, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("102", 5, kTokenTypeLiteral));
  EXPECT_EQ(tokenizer.GetNext(), Token(")", 8, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("}", 9, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.HasNext(), false);
}

TEST(Tokenizer, HandleRecurentMatch4)
{
  Tokenizer tokenizer("{FKV(102, other)}", true);
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("{", 0, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("FKV", 1, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.GetNext(), Token("(", 4, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("102", 5, kTokenTypeLiteral));
  EXPECT_EQ(tokenizer.GetNext(), Token(",", 8, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("other", 10, kTokenTypeLiteral));
  EXPECT_EQ(tokenizer.GetNext(), Token(")", 15, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("}", 16, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.HasNext(), false);
}

TEST(Tokenizer, HandleRecurentMatch5)
{
  Tokenizer tokenizer("{FKV(102, {docid}, {value})}", true);
  EXPECT_EQ(tokenizer.HasNext(), true);
  EXPECT_EQ(tokenizer.GetNext(), Token("{", 0, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("FKV", 1, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.GetNext(), Token("(", 4, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("102", 5, kTokenTypeLiteral));
  EXPECT_EQ(tokenizer.GetNext(), Token(",", 8, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("{", 10, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("docid", 11, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.GetNext(), Token("}", 16, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token(",", 17, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("{", 19, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("value", 20, kTokenTypeIdentifier));
  EXPECT_EQ(tokenizer.GetNext(), Token("}", 25, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token(")", 26, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.GetNext(), Token("}", 27, kTokenTypeSymbol));
  EXPECT_EQ(tokenizer.HasNext(), false);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
