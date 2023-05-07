#include "../src/json.cpp"	// Disgusting workaround for incomplete iterators on json.hpp
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include <fstream>
using namespace std;

#define TEST(contents, ...) \
	do { \
		stringstream stream(string((contents))); \
		((void (*)(stringstream&)) (__VA_ARGS__))(stream); \
		cout << "TEST:" << __LINE__ << ": Passed" << endl; \
	} while (false);

#define TEST_PARSER(contents) TEST((contents), [](auto s) {json j; s >> j;})

#define TEST_PARSER_THROW(contents, expected) \
	do { \
		TEST((contents), [](auto s) { \
			string msg; \
			try { \
				json j; \
				s >> j; \
			} catch (json_exception e) { msg = e.msg; } \
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
	TEST_PARSER_THROW("nullptr", "Expected EOF, got byte 112");
	TEST_PARSER("null");

	TEST_PARSER_THROW(".13", "Expected primitive, got byte 46");
	TEST_PARSER("3.14");
	TEST_PARSER("0.5e+2");
	TEST_PARSER("16.1e-1");
	TEST_PARSER("-1.61");
	TEST_PARSER("0.0");

	TEST_PARSER_THROW("f", "Expected 'false', got 'f'");
	TEST_PARSER_THROW("fals", "Expected 'false', got 'fals'");
	TEST_PARSER_THROW("flase", "Expected 'false', got 'flase'");
	TEST_PARSER_THROW("falseeeee", "Expected EOF, got byte 101");
	TEST_PARSER("false");

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
	TEST_PARSER(" \"hi\"");
	TEST_PARSER("\"\\\"\"");
	TEST_PARSER("\"some \\\\\" thing\"");	// NOTE: Standard would crash
	TEST_PARSER("\"hello\nworld\"");
	TEST_PARSER("\"some \\\"other\\\" string\"");

	TEST_PARSER_THROW("[", "Expected JSON, got EOF");
	TEST_PARSER_THROW("[[", "Expected JSON, got EOF");
	TEST_PARSER_THROW("[\"something, 3.14]", "Expected '\"', got EOF");
	TEST_PARSER_THROW("[tru]", "Expected 'true', got 'tru]'");
	TEST_PARSER_THROW("[truee]", "Expected ']', got byte 101");
	TEST_PARSER_THROW("[null, \"\", [1, 2]", "Expected ']', got EOF");
	TEST_PARSER_THROW("[,]", "Expected primitive, got byte 44");
	TEST_PARSER("[]");
	TEST_PARSER("[[[], [[[[], null], [[\"something\"]], [1]]], [false]], []]");
	TEST_PARSER("[3.14 ]");
	TEST_PARSER("[3.14, ]");
	TEST_PARSER("[\"something\", 3.14, -2, {\"\": -1}]");
	TEST_PARSER("[true, false, \"\", [1, 2]]");

	TEST_PARSER_THROW("{", "Expected '\"', got EOF");
	TEST_PARSER_THROW("{{", "Expected '\"', got byte 123");
	TEST_PARSER_THROW("{{}}", "Expected '\"', got byte 123");
	TEST_PARSER_THROW("{,}", "Expected '\"', got byte 44");
	TEST_PARSER_THROW("{\"something: 3.14}", "Expected '\"', got EOF");
	TEST_PARSER_THROW("{\"key\", \"value\"}", "Expected ':', got byte 44");
	TEST_PARSER_THROW("{\"key\": [1,}", "Expected primitive, got byte 125");
	TEST_PARSER_THROW("{\" \\\" \": [{\"a\": 1]}", "Expected '}', got byte 93");
	TEST_PARSER("{}");
	TEST_PARSER("{\"something\": \"\\\"reason\\\"\"}");
	TEST_PARSER("{\"pi\": 3.14}");
	TEST_PARSER("{\"key\": [1, {\"a\": \"b\", \"c\": 3.14}, 2], }");

	TEST("[\"first\", [2]]", [](auto s) {
		json jl;
		s >> jl;
		auto itl = jl.begin_list();

		json j1;
		j1 = *(itl++);
		{
			json j2;
			j2 = *(itl++);
			assert(itl == jl.end_list());

			assert(j2.is_list());
			auto it = j2.begin_list();
			assert(it->is_number() && it->get_number() == 2.0);
			assert((++it) == j2.end_list());

			j2 = j1;
			assert(j2.is_string() && j2.get_string() == "first");
		}
		assert(j1.is_string() && j1.get_string() == "first");
	});

	TEST("[false, \"false\"]", [](auto s) {
		json jl;
		s >> jl;
		auto it = jl.begin_list();

		json j1;
		j1 = *(it++);
		assert(j1.is_bool() && j1.get_bool() == false);
		{
			json j2;
			j2 = *(it++);
			assert(it == jl.end_list());
			assert(j2.is_string() && j2.get_string() == "false");
			j1 = move(j2);
		}
		assert(j1.is_string() && j1.get_string() == "false");
	});

	TEST("null", [](auto s) {
		json j;
		s >> j;
		assert(j.is_null());

		string msg;
		try {
			j.insert(pair<string, json>("hello", j));
		} catch (json_exception e) {
			msg = e.msg;
		}
		assert(msg == "Wrong json& type for insert");
	});

	TEST("3.14", [](auto s) {
		json j;
		s >> j;
		assert(j.is_number() && j.get_number() == 3.14);

		string msg;
		try {
			j["test"];
		} catch (json_exception e) {
			msg = e.msg;
		}
		assert(msg == "Wrong json& type for operator[]");
	});

	TEST("141.4e-2", [](auto s) {
		json j;
		s >> j;
		assert(j.is_number() && j.get_number() == 1.414);
	});

	TEST("{}", [](auto s) {
		json j;
		s >> j;
		assert(j.is_dictionary() && j.begin_dictionary() == j.end_dictionary());

		j["test"];
		auto it = j.begin_dictionary();
		assert((*it).first == "test" && (*it).second.is_null());
	});

	TEST("{}", [](auto s) {
		json j1;
		s >> j1;
		assert(j1.is_dictionary() && j1.begin_dictionary() == j1.end_dictionary());

		string msg;
		try {
			const json& j2 = j1;
			j2["test"];
		} catch (json_exception e) {
			msg = e.msg;
		}
		assert(msg == "Unable to create key 'test' for const json&");
	});

	TEST("[1,]", [](auto s) {
		json j1;
		s >> j1;
		assert(j1.is_list());

		auto it1a = j1.begin_list();
		auto it1b = it1a++;
		assert(it1a == j1.end_list() && it1b == j1.begin_list());

		json j2(j1);
		json value;
		value.set_number(2.0);
		j2.push_back(value);
		assert((++it1b) == j1.end_list());

		auto it2 = j2.begin_list();
		assert((*it2).is_number() && it2->get_number() == 1.0);
		it2++;
		assert(&(*it2) != &value);
		assert(it2->is_number() && it2->get_number() == 2.0);
	});

	TEST("[1, 2]", [](auto s) {
		json jl;
		s >> jl;
		assert(jl.is_list());

		auto it = jl.begin_list();
		json j1 = *(it++);
		assert(j1.is_number() && j1.get_number() == 1.0);

		json j2 = *(it++);
		assert(j2.is_number() && j2.get_number() == 2.0);
		assert(it == jl.end_list());

		double* j1ptr = &j1.get_number();
		double* j2ptr = &j2.get_number();
		j1 = move(j2);
		assert(&j1.get_number() == j2ptr && j2ptr != j1ptr);
	});

	TEST("", [](auto s) {
		string msg;
		try {
			json j1;
			j1.set_dictionary();
			{
				const json& j2 = j1;
				j2["missing"];
			}
		} catch (json_exception e) {
			msg = e.msg;
		}
		assert(msg == "Unable to create key 'missing' for const json&");
	});

	TEST("", [](auto s) {
		json j1;
		{
			json j2;
			j2.set_dictionary();
			j2["null"];
			j2["str"].set_string("hello");
			j1 = move(j2);
		}
		assert(j1["null"].is_null());
		assert(j1["str"].is_string() && j1["str"].get_string() == "hello");
	});

	TEST("", [](auto s) {
		json j;
		j.set_string("hi");
		assert(j.is_string() && j.get_string() == "hi");

		j.set_bool(false);
		assert(j.is_bool() && j.get_bool() == false);

		j.set_number(3.14);
		assert(j.is_number() && j.get_number() == 3.14);

		j.set_list();
		json e;
		e.set_string("world");
		j.push_back(e);
		j.push_front(j);
		assert(j.is_list());

		j.set_dictionary();
		j.insert(pair<string, json>("hello", e));
		assert(j.is_dictionary() && j["hello"].get_string() == "world");

		j.set_null();
		assert(j.is_null());
	});

	TEST("[1, 2, 3]", [](auto s) {
		json j;
		s >> j;
		assert(j.is_list());

		auto it = j.begin_list();
		assert(it->is_number() && it->get_number() == 1.0);

		(*(++it)).get_number() *= 2.0;
		assert(it->is_number() && (*(it++)).get_number() == 4.0);

		assert(it->is_number() && it->get_number() == 3.0);
	});

	TEST("1", [](auto s) {
		json j1;
		s >> j1;
		j1 = j1;
		const json& j2 = j1;
		j1 = j2;
		j1 = move(j1);
		assert(j1.is_number() && j1.get_number() == 1.0);
	});
}

int main(int argc, char** argv) {
	tests();

	// TODO:
	// Are huge datasets, like:
	// - https://github.com/jdorfman/awesome-json-datasets#historical-events (English; only with sanitizers)
	// - https://huggingface.co/datasets/enryu43/twitter100m_users
	// - https://huggingface.co/datasets/enryu43/twitter100m_tweets
	// - and others from https://www.reddit.com/r/datasets/ and https://www.reddit.com/r/opendata/
	// supposed to crash the parser of stack overflow (see valgrind)?
	// How big should the JSON file be before the parser crashes?
	for (int i = 1; i < argc; i++) {
		ifstream stream(argv[i]);

		try {
			json j;
			stream >> j;
			cout << "TEST:" << argv[i] << ": Passed" << endl;
		} catch (json_exception e) {
			cout << "TEST:" << argv[i] << ": " << e.msg << " before '";
			for (size_t j = 0; j < 50 && stream; j++) {
				char symbol = stream.get();
				cout << symbol;
			}
			cout << "'" << endl;
			break;
		}
	}
}
