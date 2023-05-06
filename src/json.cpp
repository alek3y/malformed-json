#include "json.hpp"

// Context-free grammar for the simplified JSON file type (see include/README.md):
// <Json> → <Primitive> | [<List>] | {<Dict>}
// <List> → ε | <Json> | <Json>,<List>
// <Dict> → ε | <Pair> | <Pair>,<Dict>
// <Primitive> → null | <Number> | <Bool> | "<Str>"
//
// <Pair> → ε | "<Str>":<Json>
// <Str> → ε | <Char> | <Char><Str>
// <Bool> → false | true
//
// where <Char> is a terminal with ascii from 0x20 to 0x7E with '"' = '\"',
// and <Number> is a terminal represented as a double with no leading '.'

static inline void parse_expect(std::istream& stream, std::string expected) {
	std::string content;
	content.resize(expected.length());

	size_t bytes_read = 0;
	while (stream && bytes_read < expected.length()) {
		content[bytes_read] = stream.get();

		if (stream) {
			bytes_read++;
		}
	}
	content.resize(bytes_read);

	if (content != expected) {
		std::string msg = "Expected '" + expected + "', got ";
		if (content.length() == 0 && expected.length() > 0) {
			msg += "EOF";
		} else {
			assert(content.length() > 0);
			msg += "'" + content + "'";
		}
		throw json_exception{msg};
	}
}

static std::string parse_str(std::istream& stream) {
	parse_expect(stream, "\"");

	std::string content;
	do {
		content += stream.get();
		if (!stream) {
			throw json_exception{"Expected '\"', got EOF"};
		}
	} while (content.back() != '"' || (content.length() > 1 && content[content.length()-2] == '\\'));
	content.pop_back();

	return content;
}

static void parse_primitive(std::istream& stream) {
	char symbol = 0;
	stream >> symbol;
	stream.putback(symbol);

	if (symbol == 'n') {
		parse_expect(stream, "null");
	} else if ((symbol >= '0' && symbol <= '9') || symbol == '-') {
		double number;
		stream >> number;
	} else if (symbol == 'f') {
		parse_expect(stream, "false");
	} else if (symbol == 't') {
		parse_expect(stream, "true");
	} else if (symbol == '"') {
		parse_str(stream);
	} else {
		throw json_exception{"Expected primitive, got byte " + std::to_string((int) symbol)};
	}
}

void parse(std::istream& stream);

static void parse_list(std::istream& stream) {
	char symbol = 0;
	stream >> symbol;
	stream.putback(symbol);

	if (symbol != ']') {
		parse(stream);

		stream >> symbol;
		if (symbol == ',') {
			parse_list(stream);
		} else {
			stream.putback(symbol);
		}
	}
}

static void parse_dict(std::istream& stream) {
	char symbol = 0;
	stream >> symbol;
	stream.putback(symbol);

	if (symbol != '}') {
		parse_str(stream);

		stream >> symbol;
		if (symbol != ':') {
			throw json_exception{"Expected ':', got byte " + std::to_string((int) symbol)};
		}

		parse(stream);

		stream >> symbol;
		if (symbol == ',') {
			parse_dict(stream);
		} else {
			stream.putback(symbol);
		}
	}
}

// Corresponds to the <Json> non-terminal
// TODO: Change to operator>>
void parse(std::istream& stream) {
	char symbol = 0;
	stream >> symbol;
	if (!stream) {
		throw json_exception{"Expected JSON, got EOF"};
	}

	if (symbol == '[') {
		parse_list(stream);
		parse_expect(stream, "]");
	} else if (symbol == '{') {
		parse_dict(stream);
		parse_expect(stream, "}");
	} else {
		stream.putback(symbol);
		parse_primitive(stream);
	}
}
