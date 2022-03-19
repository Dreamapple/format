// Copyright (c) 2021, Tencent Inc.
// Author: linghuimeng<linghuimeng@tencent.com>
// Create Time: 2022.03.14
// Description: 


#include "<gflags/gflags.h>"
#include "matcher.h"
DEFINE_string(format, "", "format");
DEFINE_string(source, "", "source");

using namespace fq;


int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, false);

  FormatParser parser(true);
  FormatRootNode root;
  int ret = parser.Parse(FLAGS_format, root);
  printf("Parse %s ret=%d\n", FLAGS_format.c_str(), ret);

  root.Dump(0);

  MatchResult result;
  bool h = root.Handle(FLAGS_source, 0, FLAGS_source.size(), result);
  printf("handle result = %d\n", h);
  result.Dump();

  return 0;
}





