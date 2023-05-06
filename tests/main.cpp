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
		cout << "TEST:" << __LINE__ << ": Passed" << endl; \
	} while (false);

#define TEST_PARSER(contents) TEST((contents), [](auto s) {parse(s);})

#define TEST_PARSER_THROW(contents, expected) \
	do { \
		TEST((contents), [](auto s) { \
			string msg; \
			try { parse(s); } catch (json_exception e) { msg = e.msg; } \
			if (msg != (expected)) { \
				cout << "TEST:" << __LINE__ << ": e.msg: " << msg << endl; \
			} \
			assert(msg == (expected)); \
		}); \
	} while (false);

void tests(void) {
	TEST_PARSER_THROW("", "Unexpected EOF");
	TEST_PARSER_THROW("A", "Expected primitive, got byte 65");
	TEST_PARSER_THROW("nul", "Expected 'null', got 'nul'");
	TEST_PARSER_THROW("nulu", "Expected 'null', got 'nulu'");
	TEST_PARSER("null");

	TEST_PARSER("3.14");

	TEST_PARSER_THROW("f", "Expected 'false', got 'f'");
	TEST_PARSER_THROW("fals", "Expected 'false', got 'fals'");
	TEST_PARSER_THROW("flase", "Expected 'false', got 'flase'");
	TEST_PARSER("false");

	TEST_PARSER("falseeeee");	// TODO: Should this work?

	TEST_PARSER_THROW("t", "Expected 'true', got 't'");
	TEST_PARSER_THROW("tre", "Expected 'true', got 'tre'");
	TEST_PARSER_THROW("treu", "Expected 'true', got 'treu'");
	TEST_PARSER("true");

	TEST_PARSER_THROW("\"", "Expected '\"', got EOF");
	TEST_PARSER_THROW("\"something", "Expected '\"', got EOF");
	TEST_PARSER_THROW("\"\\\"", "Expected '\"', got EOF");
	TEST_PARSER_THROW("\"something\\\"", "Expected '\"', got EOF");
	TEST_PARSER("\"\"");
	TEST_PARSER("\"\\\"\"");
	TEST_PARSER("\"some \\\\\" thing\"");
	TEST_PARSER("\"hello\nworld\"");
	TEST_PARSER("\"some \\\"other\\\" string\"");

	TEST_PARSER_THROW("[", "Unexpected EOF");
	TEST_PARSER_THROW("[[", "Unexpected EOF");
	TEST_PARSER_THROW("[\"something, 3.14]", "Expected '\"', got EOF");
	TEST_PARSER_THROW("[tru]", "Expected 'true', got 'tru]'");
	TEST_PARSER_THROW("[null, \"\", [1, 2]", "Expected ']', got ''");
	TEST_PARSER_THROW("[,]", "Expected primitive, got byte 44");
	TEST_PARSER("[]");
	TEST_PARSER("[3.14]");
	TEST_PARSER("[3.14,]");
	TEST_PARSER("[\"something\", 3.14]");
	TEST_PARSER("[true, false, \"\", [1, 2]]");
}

int main(void) {
	tests();
}
