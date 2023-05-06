#include "json.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
using namespace std;

void parse(istream&);

#define TEST(contents, lambda) \
	do { \
		stringstream stream(string((contents))); \
		((void (*)(stringstream&)) (lambda))(stream); \
		cout << "TEST:" << __LINE__ << ": Success" << endl; \
	} while (false);

#define TEST_PARSER(contents) \
	do { \
		TEST((contents), [](auto s) {parse(s);}); \
	} while (false);

#define TEST_PARSER_THROW(contents, expected) \
	do { \
		TEST((contents), [](auto s) { \
			string msg; \
			try { parse(s); } catch (json_exception e) { msg = e.msg; } \
			if (msg != (expected)) { \
				cout << "e.msg: '" << msg << "'" << endl; \
			} \
			assert(msg == (expected)); \
		}); \
	} while (false);

void tests(void) {
	TEST_PARSER_THROW("", "Unexpected empty file");
	TEST_PARSER_THROW("A", "Expected primitive, but found byte 65");
	TEST_PARSER_THROW("nul", "Expected 'null', but got 'nul'");
	TEST_PARSER_THROW("nulu", "Expected 'null', but got 'nulu'");
	TEST_PARSER("null");

	TEST_PARSER("3.14");

	TEST_PARSER_THROW("f", "Expected 'false', but got 'f'");
	TEST_PARSER_THROW("fals", "Expected 'false', but got 'fals'");
	TEST_PARSER_THROW("flase", "Expected 'false', but got 'flase'");
	TEST_PARSER("false");

	TEST_PARSER_THROW("t", "Expected 'true', but got 't'");
	TEST_PARSER_THROW("tre", "Expected 'true', but got 'tre'");
	TEST_PARSER_THROW("treu", "Expected 'true', but got 'treu'");
	TEST_PARSER("true");

	TEST_PARSER_THROW("\"", "Expected end of string, got EOF");
	TEST_PARSER_THROW("\"something", "Expected end of string, got EOF");
	TEST_PARSER_THROW("\"\\\"", "Expected end of string, got EOF");
	TEST_PARSER_THROW("\"something\\\"", "Expected end of string, got EOF");
	TEST_PARSER("\"\"");
	TEST_PARSER("\"\\\"\"");
	TEST_PARSER("\"some \\\\\" thing\"");
	TEST_PARSER("\"hello\nworld\"");
	TEST_PARSER("\"some \\\"other\\\" string\"");
}

int main(void) {
	tests();
}
