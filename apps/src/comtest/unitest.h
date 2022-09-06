/**
* BSD 3-Clause License
* 
* Copyright (c) 2019, Caswall Engelsman <caswall@kickness.org>
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
*   * Redistributions of source code must retain the above copyright notice, this
*     list of conditions and the following disclaimer.
* 
*   * Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
* 
*   * Neither the name of the copyright holder nor the names of its
*     contributors may be used to endorse or promote products derived from
*     this software without specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UNITEST
#define UNITEST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define T_SETUP_RESULT_SIZE (10)
#define T_SUITE_SETUP_RESULT_SIZE (10)

#define T_FAIL_COLOUR (31)
#define T_PASS_COLOUR (32)
#define T_GREY_COLOUR (90)

#define _T_DO_NOTHING do {} while(0)

#ifdef T_REPORTER_LIST
	#define _T_SUITE_TITLE(msg) _T_DO_NOTHING

	#define _T_TEST_FAIL_MSG(msg_format, a, b) \
		char format[] = "[%s] %s:%d: "#msg_format"\n"; \
		T_SET_COLOUR(stderr, T_FAIL_COLOUR); \
		fprintf(stderr, "FAIL"); \
		T_SET_COLOUR(stderr, T_GREY_COLOUR); \
		fprintf(stderr, " %s: ", _suite_title); \
		T_FAIL_DEBUG_MSG(format, a, b); \
		T_RESET_COLOUR(stderr)

	#define _T_TEST_PASS_MSG() \
		T_SET_COLOUR(stdout, T_PASS_COLOUR); \
		fprintf(stdout, "PASS"); \
		T_SET_COLOUR(stdout, T_GREY_COLOUR); \
		fprintf(stdout, " %s %s\n", _suite_title, _test_title); \
		T_RESET_COLOUR(stdout)

	#define _T_CONCLUDE() \
		if(T_PASSED < T_COUNT){ \
			fprintf(stderr, "\n\n"); \
			T_SET_COLOUR(stderr, T_FAIL_COLOUR); \
			fprintf(stderr, "%d of %d tests failed\n", T_COUNT - T_PASSED, T_COUNT); \
			T_RESET_COLOUR(stderr); \
			return 1; \
		} \
		fprintf(stdout, "\n\n"); \
		T_SET_COLOUR(stdout, T_PASS_COLOUR); \
		fprintf(stdout, "%d tests completed\n", T_PASSED); \
		T_RESET_COLOUR(stdout)

#elif T_REPORTER_DOT
	#define _T_SUITE_TITLE(msg) _T_DO_NOTHING

	#define _T_TEST_FAIL_MSG(msg, a, b) \
		T_SET_COLOUR(stderr, T_FAIL_COLOUR); \
		fprintf(stderr, "!"); \
		T_RESET_COLOUR(stderr)

	#define _T_TEST_PASS_MSG() \
		T_SET_COLOUR(stdout, T_GREY_COLOUR); \
		fprintf(stdout, "."); \
		T_RESET_COLOUR(stdout)

	#define _T_CONCLUDE() \
		fprintf(stdout, "\n\n"); \
		T_SET_COLOUR(stdout, T_PASS_COLOUR); \
		fprintf(stdout, "%d passing\n", T_PASSED); \
		T_RESET_COLOUR(stdout); \
		if(T_PASSED < T_COUNT){ \
			T_SET_COLOUR(stderr, T_FAIL_COLOUR); \
			fprintf(stderr, "%d failing\n", T_COUNT - T_PASSED); \
			T_RESET_COLOUR(stderr); \
			return 1; \
		}
#else
	/** T_REPORTER_SPEC as default */
	#define _T_SUITE_TITLE(msg) \
		_PRINTF_INDENT(stdout); \
		fprintf(stdout, "%s\n", msg)

	#define _T_TEST_FAIL_MSG(msg_format, a, b) \
		char format[] = "FAIL [%s] %s:%d: "#msg_format"\n"; \
		T_SET_COLOUR(stderr, T_FAIL_COLOUR); \
		_PRINTF_INDENT(stderr); \
		T_FAIL_DEBUG_MSG(format, a, b); \
		T_RESET_COLOUR(stderr)

	#define _T_TEST_PASS_MSG() \
		T_SET_COLOUR(stdout, T_PASS_COLOUR); \
		_PRINTF_INDENT(stdout); \
		fprintf(stdout, "PASS"); \
		T_RESET_COLOUR(stdout); \
		fprintf(stdout, " %s\n", _test_title)

	#define _T_CONCLUDE() \
		if(T_PASSED < T_COUNT){ \
			T_SET_COLOUR(stderr, T_FAIL_COLOUR); \
			fprintf(stderr, "FAIL %d of %d tests failed\n", T_COUNT - T_PASSED, T_COUNT); \
			T_RESET_COLOUR(stderr); \
			return 1; \
		} \
		T_SET_COLOUR(stdout, T_PASS_COLOUR); \
		fprintf(stdout, "PASS %d tests completed\n", T_PASSED); \
		T_RESET_COLOUR(stdout)
#endif

#define T_SUITE(title, code) \
	_T_SUITE_TITLE(#title); \
	T_PRINT_LEVEL++;  \
	{ \
		char _title[] = #title; \
		void** SUITE_SETUP_RESULT = NULL; \
		_suite_title = _title; \
		if(T_SUITE_SETUP_FUNC){ \
			SUITE_SETUP_RESULT = (void**)malloc(sizeof(void*) * T_SUITE_SETUP_RESULT_SIZE); \
			T_SUITE_SETUP_FUNC(SUITE_SETUP_RESULT); \
			T_SUITE_SETUP_FUNC = 0; \
		} \
		{code;} \
		if(T_SUITE_TEARDOWN_FUNC){ \
			T_SUITE_TEARDOWN_FUNC(SUITE_SETUP_RESULT); \
			T_SUITE_TEARDOWN_FUNC = 0; \
		} \
		_suite_title = NULL; /** reset suite title */ \
		free(SUITE_SETUP_RESULT); \
	} \
	T_PRINT_LEVEL--

#define T_FAILED(msg, a, b) \
	_T_TEST_FAIL_MSG(msg, a, b); \
	T_FLAG = 1

#define T_PASSED() \
	_T_TEST_PASS_MSG(); \
	T_PASSED++

#define TEST(title, code) NEG_FLAG = 1; MAIN_TEST(title, code)
#define N_TEST(title, code) NEG_FLAG = 0; MAIN_TEST(title, code)

#define MAIN_TEST(title, code) T_FLAG = 0; T_COUNT++; \
	{ \
		char _title[] = #title; \
		void** T_SETUP_RESULT = NULL; \
        _test_title = _title; \
		if(T_SETUP_FUNC){ \
			T_SETUP_RESULT = (void**)malloc(sizeof(void*) * T_SETUP_RESULT_SIZE); \
			T_SETUP_FUNC(T_SETUP_RESULT); \
		} \
		{code;} \
		if(T_TEARDOWN_FUNC){ T_TEARDOWN_FUNC(T_SETUP_RESULT); } \
		free(T_SETUP_RESULT); \
		if(!T_FLAG){ \
			T_PASSED(); \
		} \
		_test_title = NULL; /** reset test title */ \
	}

#define ASSERT(statement, op1, op2, format) \
	if(!!(statement) ^ NEG_FLAG){ T_FAILED(format, op1, op2); }

#define T_ASSERT(statement) \
	ASSERT(statement, #statement, "", "%s%s")

#define T_ASSERT_STRING(a, b) \
	ASSERT(!strcmp((a), (b)), (a), (b), "%s != %s")

#define T_ASSERT_CHAR(a, b) \
	ASSERT((a) == (b), (a), (b), "%c != %c")

#define T_ASSERT_NUM(a, b) \
	if(sizeof((a)) <= sizeof(int)){ \
		ASSERT((a) == (b), (a), (b), "%d != %d"); \
	}else if(sizeof((a)) == sizeof(long)){ \
		ASSERT((a) == (b), (a), (b), "%ld != %ld"); \
	}

#define T_ASSERT_FLOAT(a, b) \
	ASSERT((a) == (b), (a), (b), "%f != %f")

#define _PRINTF_INDENT(stream) \
	{	int i = T_PRINT_LEVEL; \
		while(i--){ fprintf(stream, "   "); } }
#define T_SET_COLOUR(stream, colour) fprintf(stream, "\033[1;%dm", colour)
#define T_RESET_COLOUR(stream) fprintf(stream, "\033[0m")
#define T_FAIL_DEBUG_MSG(format, a, b) \
	fprintf(stderr, format, _test_title, __FILE__, __LINE__, (a), (b))

#define T_SETUP(func) T_SETUP_FUNC = func
#define T_TEARDOWN(func) T_TEARDOWN_FUNC = func
#define T_SUITE_SETUP(func) T_SUITE_SETUP_FUNC = func
#define T_SUITE_TEARDOWN(func) T_SUITE_TEARDOWN_FUNC = func
#define T_CONCLUDE _T_CONCLUDE

typedef void (*_test_function_t)(void**);
char T_FLAG, NEG_FLAG, *_test_title, *_suite_title;
int T_COUNT = 0, T_PASSED = 0, T_PRINT_LEVEL = 0;
_test_function_t T_SETUP_FUNC = 0;
_test_function_t T_SUITE_SETUP_FUNC = 0;
_test_function_t T_TEARDOWN_FUNC = 0;
_test_function_t T_SUITE_TEARDOWN_FUNC = 0;

#endif
