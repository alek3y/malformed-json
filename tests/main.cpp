#include "json.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <functional>
using namespace std;

void parse(istream&);

void test(string contents, bool throws, bool (*lambda)(std::istream&), unsigned line) {
	string msg;
	bool success;
	try {
		stringstream stream(contents);
		success = lambda(stream) && !throws;
	} catch (json_exception e) {
		msg = e.msg;
		success = throws;
	}

	cout << "TEST:" << line << ": ";
	if (success) {
		cout << "Success";
	} else {
		cout << "Fail";
	}

	if (msg.length() > 0) {
		cout << " => " << msg;
	}
	cout << endl;
}

void test_parse(string contents, bool throws, unsigned line) {
	test(contents, throws, [](auto s) -> bool {
		parse(s);
		return true;
	}, line);
}

#define TEST(contents, throws, lambda) test(contents, throws, lambda, __LINE__)
#define TEST_PARSE(contents, throws) test_parse(contents, throws, __LINE__)

int main(void) {
	TEST_PARSE("", true);
	TEST_PARSE("nul", true);
	TEST_PARSE("null", false);
}
