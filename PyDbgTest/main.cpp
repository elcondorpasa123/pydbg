#include "pytest.h"
#include "assert.h"

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")


char* const TEST_PYFRAME_CODE		= "!pyframe 068afde0";

char* const TEST_PYFRAME_RESULT		= "\
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
char* const TEST_PYOBJ_INT_RESULT = "161";

namespace test{
	class TestObj
	{};
};

int main()
{
	::test::TestObj obj;
	assert(0 == pydll_exec_unittest(TEST_PYFRAME_CODE, TEST_PYFRAME_RESULT));
	assert(0 == pydll_exec_unittest(TEST_PYOBJ_STR_CODE, TEST_PYOBJ_STR_RESULT));
	assert(0 == pydll_exec_unittest(TEST_PYOBJ_INT_CODE, TEST_PYOBJ_INT_RESULT));
	return 0;
}

