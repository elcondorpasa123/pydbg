// Copyright 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
#include "../PyDbgTest/pytest.h"
#include "gtest/gtest.h"
namespace {

char* const TEST_PYFRAME_CODE	= "!pyframe 068afde0";

char* const TEST_PYFRAME_RESULT	= "\
										  \"C:\\CCCodes\\apps_wonderful\\apps\\template_process\\unity_proxy.py\"\
										  line: 96\
										  \"C:\\CCCodes\\apps_wonderful\\apps\\unity\\__init__.py\"\
										  line: 51\
										  \"C:/CCCodes/apps_wonderful/apps/room_debug.py\"\
										  line: 107\
										  \"C:/CCCodes/apps_wonderful/apps/room_debug.py\"\
										  line: 129\
										  \"C:\\Program Files\\JetBrains\\PyCharm 2018.1.4\\helpers\\pydev\\pydevd.py\"\
										  line: 1068\
										  \"C:\\Program Files\\JetBrains\\PyCharm 2018.1.4\\helpers\\pydev\\pydevd.py\"\
										  line: 1658\
										  \"C:\\Program Files\\JetBrains\\PyCharm 2018.1.4\\helpers\\pydev\\pydevd.py\"\
										  line: 1664";

char* const TEST_PYOBJ_STR_CODE = "!pyobj 0x069a2a70";
char* const TEST_PYOBJ_STR_RESULT = "\"C:\\CCCodes\\apps_wonderful\\apps\\template_process\\unity_proxy.py\"";

char* const TEST_PYOBJ_INT_CODE = "!pyobj 0x05d299d0";
char* const TEST_PYOBJ_INT_RESULT = "160"; // 161

TEST(pydll_exec_unittest, pyobj) {
  EXPECT_EQ(0, pydll_exec_unittest(TEST_PYOBJ_STR_CODE, TEST_PYOBJ_STR_RESULT));
  EXPECT_EQ(0, pydll_exec_unittest(TEST_PYOBJ_INT_CODE, TEST_PYOBJ_INT_RESULT))  << "error !pyobj inttype";

}

TEST(pydll_exec_unittest, pyframe) {
  EXPECT_EQ(0, pydll_exec_unittest(TEST_PYFRAME_CODE, TEST_PYFRAME_RESULT));
}
}  // namespace