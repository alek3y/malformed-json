#include "json.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include <fstream>
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
	TEST_PARSER_THROW("", "Expected JSON, got EOF");
	TEST_PARSER_THROW("A", "Expected primitive, got byte 65");
	TEST_PARSER_THROW("nul", "Expected 'null', got 'nul'");
	TEST_PARSER_THROW("nulu", "Expected 'null', got 'nulu'");
	TEST_PARSER("null");

	TEST_PARSER_THROW(".13", "Expected primitive, got byte 46");
	TEST_PARSER("3.14");
	TEST_PARSER("-1.61");
	TEST_PARSER("0.0");

	TEST_PARSER_THROW("f", "Expected 'false', got 'f'");
	TEST_PARSER_THROW("fals", "Expected 'false', got 'fals'");
	TEST_PARSER_THROW("flase", "Expected 'false', got 'flase'");
	TEST_PARSER("false");

	// TODO: Which one?
	//TEST_PARSER_THROW("falseeeee", "Expected EOF, got byte 101");
	TEST_PARSER("falseeeee");

	TEST_PARSER_THROW("t", "Expected 'true', got 't'");
	TEST_PARSER_THROW("tre", "Expected 'true', got 'tre'");
	TEST_PARSER_THROW("treu", "Expected 'true', got 'treu'");
	TEST_PARSER("true");

	TEST_PARSER_THROW("\"", "Expected '\"', got EOF");
	TEST_PARSER_THROW("\"something", "Expected '\"', got EOF");
	TEST_PARSER_THROW("\"\\\"", "Expected '\"', got EOF");
	TEST_PARSER_THROW("\"something\\\"", "Expected '\"', got EOF");
	TEST_PARSER_THROW("\"something\\\\\"", "Expected '\"', got EOF");	// NOTE: Standard wouldn't crash
	TEST_PARSER("\"\"");
	TEST_PARSER("\"\\\"\"");
	TEST_PARSER("\"some \\\\\" thing\"");	// NOTE: Standard would crash
	TEST_PARSER("\"hello\nworld\"");
	TEST_PARSER("\"some \\\"other\\\" string\"");

	TEST_PARSER_THROW("[", "Expected JSON, got EOF");
	TEST_PARSER_THROW("[[", "Expected JSON, got EOF");
	TEST_PARSER_THROW("[\"something, 3.14]", "Expected '\"', got EOF");
	TEST_PARSER_THROW("[tru]", "Expected 'true', got 'tru]'");
	TEST_PARSER_THROW("[truee]", "Expected ']', got 'e'");
	TEST_PARSER_THROW("[null, \"\", [1, 2]", "Expected ']', got EOF");
	TEST_PARSER_THROW("[,]", "Expected primitive, got byte 44");
	TEST_PARSER("[]");
	TEST_PARSER("[[[], [[[[], null], [[\"something\"]], [1]]], [false]], []]");
	TEST_PARSER("[3.14]");
	TEST_PARSER("[3.14,]");
	TEST_PARSER("[\"something\", 3.14, -2, {\"\": -1}]");
	TEST_PARSER("[true, false, \"\", [1, 2]]");

	TEST_PARSER_THROW("{", "Expected '\"', got EOF");
	TEST_PARSER_THROW("{{", "Expected '\"', got '{'");
	TEST_PARSER_THROW("{{}}", "Expected '\"', got '{'");
	TEST_PARSER_THROW("{,}", "Expected '\"', got ','");
	TEST_PARSER_THROW("{\"something: 3.14}", "Expected '\"', got EOF");
	TEST_PARSER_THROW("{\"key\", \"value\"}", "Expected ':', got byte 44");
	TEST_PARSER_THROW("{\"key\": [1,}", "Expected primitive, got byte 125");
	TEST_PARSER_THROW("{\" \\\" \": [{\"a\": 1]}", "Expected '}', got ']'");
	TEST_PARSER("{}");
	TEST_PARSER("{\"something\": \"\\\"reason\\\"\"}");
	TEST_PARSER("{\"pi\": 3.14}");
	TEST_PARSER("{\"key\": [1, {\"a\": \"b\", \"c\": 3.14}, 2], }");
}

int main(int argc, char** argv) {
	tests();

	for (int i = 1; i < argc; i++) {
		ifstream stream(argv[i]);

		try {
			parse(stream);
			cout << "TEST:" << argv[i] << ": Passed" << endl;
		} catch (json_exception e) {
			cout << "TEST:" << argv[i] << ": Exception (" << e.msg << ") before '";
			for (size_t j = 0; j < 20 && stream; j++) {
				char symbol = stream.get();
				cout << symbol;
			}
			if (stream) {
				cout << "...";
			}
			cout << "'" << endl;
			break;
		}
	}
}
