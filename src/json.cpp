#include "json.hpp"

struct json::impl {
	enum json_type {
		JSON_NULL,
		JSON_NUMBER,
		JSON_BOOL,
		JSON_STR,
		JSON_LIST,
		JSON_DICT
	} type;

	double n;
	bool b;
	std::string s;

	struct list {
		std::pair<std::string, json> value;
		list *next;
	};
	list* head;
	list* tail;

	impl() : type(JSON_NULL), s(), head(nullptr), tail(nullptr) {}

	impl(const impl& rhs) {
		*this = rhs;
	}

	~impl() {
		clear();
	}

	void clear() {
		type = JSON_NULL;
		s.clear();
		while (head != nullptr) {
			list* previous = head;
			head = head->next;
			delete previous;
		}
		tail = nullptr;
	}

	impl& operator=(const impl& rhs) {
		clear();
		type = rhs.type;

		n = rhs.n;
		b = rhs.b;
		s = rhs.s;

		list* ptr = rhs.head;
		while (ptr != nullptr) {
			push_back(ptr->value);
			ptr = ptr->next;
		}
		return *this;
	}

	void push_back(const std::pair<std::string, json>& value) {
		list* node = new list;
		node->value = value;
		node->next = nullptr;

		if (tail == nullptr) {
			head = node;
		} else {
			tail->next = node;
		}
		tail = node;
	}

	void push_front(const std::pair<std::string, json>& value) {
		list* node = new list;
		node->value = value;
		node->next = head;

		if (head == nullptr) {
			tail = node;
		}
		head = node;
	}

	list* get(const std::string& key) {
		list* ptr = head;
		while (ptr != nullptr && ptr->value.first != key) {
			ptr = ptr->next;
		}
		return ptr;
	}
};

json::json() {
	pimpl = new impl;
}

json::json(const json& rhs) : json() {
	*this = rhs;
}

json::json(json&& rhs) : json() {
	*this = std::move(rhs);
}

json::~json() {
	delete pimpl;
}

json& json::operator=(const json& rhs) {
	if (this != &rhs) {
		*pimpl = *rhs.pimpl;
	}
	return *this;
}

json& json::operator=(json&& rhs) {
	if (this != &rhs) {
		delete pimpl;
		pimpl = rhs.pimpl;
		rhs.pimpl = nullptr;
	}
	return *this;
}

bool json::is_list() const {
	return pimpl->type == impl::json_type::JSON_LIST;
}

bool json::is_dictionary() const {
	return pimpl->type == impl::json_type::JSON_DICT;
}

bool json::is_string() const {
	return pimpl->type == impl::json_type::JSON_STR;
}

bool json::is_number() const {
	return pimpl->type == impl::json_type::JSON_NUMBER;
}

bool json::is_bool() const {
	return pimpl->type == impl::json_type::JSON_BOOL;
}

bool json::is_null() const {
	return pimpl->type == impl::json_type::JSON_NULL;
}

const json& json::operator[](const std::string& rhs) const {
	if (!is_dictionary()) {
		throw json_exception{"Wrong const json& type for operator[]"};
	}

	impl::list* node = pimpl->get(rhs);
	if (node == nullptr) {
		throw json_exception{"Unable to create key '" + rhs + "' for const json&"};
	}
	return node->value.second;
}

json& json::operator[](const std::string& rhs) {
	if (!is_dictionary()) {
		throw json_exception{"Wrong json& type for operator[]"};
	}

	impl::list* node = pimpl->get(rhs);
	if (node == nullptr) {
		std::pair<std::string, json> pair(rhs, json());
		insert(pair);
		return pimpl->tail->value.second;
	}
	return node->value.second;
}

double& json::get_number() {
	if (!is_number()) {
		throw json_exception{"Wrong json& type for get_number"};
	}
	return pimpl->n;
}

const double& json::get_number() const {
	if (!is_number()) {
		throw json_exception{"Wrong const json& type for get_number"};
	}
	return pimpl->n;
}

bool& json::get_bool() {
	if (!is_bool()) {
		throw json_exception{"Wrong json& type for get_bool"};
	}
	return pimpl->b;
}

const bool& json::get_bool() const {
	if (!is_bool()) {
		throw json_exception{"Wrong const json& type for get_bool"};
	}
	return pimpl->b;
}

std::string& json::get_string() {
	if (!is_string()) {
		throw json_exception{"Wrong json& type for get_string"};
	}
	return pimpl->s;
}

const std::string& json::get_string() const {
	if (!is_string()) {
		throw json_exception{"Wrong const json& type for get_string"};
	}
	return pimpl->s;
}

void json::set_string(const std::string& string) {
	pimpl->clear();
	pimpl->s = string;
	pimpl->type = impl::json_type::JSON_STR;
}

void json::set_bool(bool boolean) {
	pimpl->clear();
	pimpl->b = boolean;
	pimpl->type = impl::json_type::JSON_BOOL;
}

void json::set_number(double number) {
	pimpl->clear();
	pimpl->n = number;
	pimpl->type = impl::json_type::JSON_NUMBER;
}

void json::set_null() {
	pimpl->clear();
}

void json::set_list() {
	pimpl->clear();
	pimpl->type = impl::json_type::JSON_LIST;
}

void json::set_dictionary() {
	pimpl->clear();
	pimpl->type = impl::json_type::JSON_DICT;
}

void json::push_front(const json& rhs) {
	if (!is_list()) {
		throw json_exception{"Wrong json& type for push_front"};
	}

	pimpl->push_front(std::pair<std::string, json>(std::string(), rhs));
}

void json::push_back(const json& rhs) {
	if (!is_list()) {
		throw json_exception{"Wrong json& type for push_back"};
	}

	pimpl->push_back(std::pair<std::string, json>(std::string(), rhs));
}

void json::insert(const std::pair<std::string, json>& rhs) {
	if (!is_dictionary()) {
		throw json_exception{"Wrong json& type for insert"};
	}

	pimpl->push_back(rhs);
}

struct json::list_iterator {
	using iterator_category = std::forward_iterator_tag;
	using value_type = json;
	using pointer = value_type*;
	using reference = value_type&;

	list_iterator(impl::list* node) : ptr(node) {}

	reference operator*() const {
		return ptr->value.second;
	}

	pointer operator->() const {
		return &ptr->value.second;
	}

	list_iterator& operator++() {
		ptr = ptr->next;
		return *this;
	}

	list_iterator operator++(int) {
		list_iterator it(ptr);
		++(*this);
		return it;
	}

	bool operator==(const list_iterator& rhs) const {
		return ptr == rhs.ptr;
	}

	bool operator!=(const list_iterator& rhs) const {
		return ptr != rhs.ptr;
	}

	operator bool() const {
		return ptr != nullptr;
	}

	private:
		impl::list* ptr;
};

json::list_iterator json::begin_list() {
	if (!is_list()) {
		throw json_exception{"Wrong json& type for begin_list"};
	}

	return list_iterator(pimpl->head);
}

json::list_iterator json::end_list() {
	if (!is_list()) {
		throw json_exception{"Wrong json& type for end_list"};
	}

	return list_iterator(nullptr);
}

struct json::const_list_iterator {
	using iterator_category = std::forward_iterator_tag;
	using value_type = const json;
	using pointer = value_type*;
	using reference = value_type&;

	const_list_iterator(impl::list* node) : ptr(node) {}

	reference operator*() const {
		return ptr->value.second;
	}

	pointer operator->() const {
		return &ptr->value.second;
	}

	const_list_iterator& operator++() {
		ptr = ptr->next;
		return *this;
	}

	const_list_iterator operator++(int) {
		const_list_iterator it(ptr);
		++(*this);
		return it;
	}

	bool operator==(const const_list_iterator& rhs) const {
		return ptr == rhs.ptr;
	}

	bool operator!=(const const_list_iterator& rhs) const {
		return ptr != rhs.ptr;
	}

	operator bool() const {
		return ptr != nullptr;
	}

	private:
		impl::list* ptr;
};

json::const_list_iterator json::begin_list() const {
	if (!is_list()) {
		throw json_exception{"Wrong const json& type for begin_list"};
	}

	return const_list_iterator(pimpl->head);
}

json::const_list_iterator json::end_list() const {
	if (!is_list()) {
		throw json_exception{"Wrong const json& type for end_list"};
	}

	return const_list_iterator(nullptr);
}

struct json::dictionary_iterator {
	using iterator_category = std::forward_iterator_tag;
	using value_type = std::pair<std::string, json>;
	using pointer = value_type*;
	using reference = value_type&;

	dictionary_iterator(impl::list* node) : ptr(node) {}

	reference operator*() const {
		return ptr->value;
	}

	pointer operator->() const {
		return &ptr->value;
	}

	dictionary_iterator& operator++() {
		ptr = ptr->next;
		return *this;
	}

	dictionary_iterator operator++(int) {
		dictionary_iterator it(ptr);
		++(*this);
		return it;
	}

	bool operator==(const dictionary_iterator& rhs) const {
		return ptr == rhs.ptr;
	}

	bool operator!=(const dictionary_iterator& rhs) const {
		return ptr != rhs.ptr;
	}

	operator bool() const {
		return ptr != nullptr;
	}

	private:
		impl::list* ptr;
};

json::dictionary_iterator json::begin_dictionary() {
	if (!is_dictionary()) {
		throw json_exception{"Wrong json& type for begin_dictionary"};
	}

	return dictionary_iterator(pimpl->head);
}

json::dictionary_iterator json::end_dictionary() {
	if (!is_dictionary()) {
		throw json_exception{"Wrong json& type for end_dictionary"};
	}

	return dictionary_iterator(nullptr);
}

struct json::const_dictionary_iterator {
	using iterator_category = std::forward_iterator_tag;
	using value_type = const std::pair<std::string, json>;
	using pointer = value_type*;
	using reference = value_type&;

	const_dictionary_iterator(impl::list* node) : ptr(node) {}

	reference operator*() const {
		return ptr->value;
	}

	pointer operator->() const {
		return &ptr->value;
	}

	const_dictionary_iterator& operator++() {
		ptr = ptr->next;
		return *this;
	}

	const_dictionary_iterator operator++(int) {
		const_dictionary_iterator it(ptr);
		++(*this);
		return it;
	}

	bool operator==(const const_dictionary_iterator& rhs) const {
		return ptr == rhs.ptr;
	}

	bool operator!=(const const_dictionary_iterator& rhs) const {
		return ptr != rhs.ptr;
	}

	operator bool() const {
		return ptr != nullptr;
	}

	private:
		impl::list* ptr;
};

json::const_dictionary_iterator json::begin_dictionary() const {
	if (!is_dictionary()) {
		throw json_exception{"Wrong const json& type for begin_dictionary"};
	}

	return const_dictionary_iterator(pimpl->head);
}

json::const_dictionary_iterator json::end_dictionary() const {
	if (!is_dictionary()) {
		throw json_exception{"Wrong const json& type for end_dictionary"};
	}

	return const_dictionary_iterator(nullptr);
}

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
	content.resize(expected.size());

	size_t bytes_read = 0;
	while (stream && bytes_read < expected.size()) {
		content[bytes_read] = stream.get();

		if (stream) {
			bytes_read++;
		}
	}
	content.resize(bytes_read);

	if (content != expected) {
		std::string msg = "Expected '" + expected + "', got ";
		if (content.size() == 0 && expected.size() > 0) {
			msg += "EOF";
		} else {
			msg += "'" + content + "'";
		}
		throw json_exception{msg};
	}
}

static inline void parse_expect(std::istream& stream, char expected) {
	char content = 0;
	stream >> content;
	if (content != expected) {
		std::string msg = "Expected '" + std::string(1, expected) + "', got ";
		if (!stream) {
			msg += "EOF";
		} else {
			msg += "byte " + std::to_string((int) content);
		}
		throw json_exception{msg};
	}
}

static std::string parse_str(std::istream& stream) {
	parse_expect(stream, '"');

	std::string content;
	do {
		content += stream.get();
		if (!stream) {
			throw json_exception{"Expected '\"', got EOF"};
		}
	} while (content.back() != '"' || (content.size() > 1 && content[content.size()-2] == '\\'));
	content.pop_back();

	return content;
}

static void parse_primitive(std::istream& stream, json& container) {
	char symbol = 0;
	stream >> symbol;
	stream.putback(symbol);

	if (symbol == 'n') {
		parse_expect(stream, "null");
		container.set_null();
	} else if ((symbol >= '0' && symbol <= '9') || symbol == '-') {
		double number;
		stream >> number;
		container.set_number(number);
	} else if (symbol == 'f') {
		parse_expect(stream, "false");
		container.set_bool(false);
	} else if (symbol == 't') {
		parse_expect(stream, "true");
		container.set_bool(true);
	} else if (symbol == '"') {
		container.set_string(parse_str(stream));
	} else {
		throw json_exception{"Expected primitive, got byte " + std::to_string((int) symbol)};
	}
}

// TODO: Refactor this?
static void parse_list(std::istream& stream, json& container) {
	bool end_of_list = false;
	while (!end_of_list) {
		char symbol = 0;
		stream >> symbol;
		stream.putback(symbol);

		if (symbol != ']') {
			json element;
			stream >> element;
			container.push_back(element);

			stream >> symbol;
			if (symbol != ',') {
				stream.putback(symbol);
				end_of_list = true;
			}
		} else {
			end_of_list = true;
		}
	}
}

static void parse_dict(std::istream& stream, json& container) {
	bool end_of_dict = false;
	while (!end_of_dict) {
		char symbol = 0;
		stream >> symbol;
		stream.putback(symbol);

		if (symbol != '}') {
			std::string key = parse_str(stream);
			parse_expect(stream, ':');

			json value;
			stream >> value;
			container.insert(std::pair<std::string, json>(key, value));

			stream >> symbol;
			if (symbol != ',') {
				stream.putback(symbol);
				end_of_dict = true;
			}
		} else {
			end_of_dict = true;
		}
	}
}

// Corresponds to the <Json> non-terminal
std::istream& operator>>(std::istream& lhs, json& rhs) {
	char symbol = 0;
	lhs >> symbol;
	if (!lhs) {
		throw json_exception{"Expected JSON, got EOF"};
	}

	if (symbol == '[') {
		rhs.set_list();
		parse_list(lhs, rhs);
		parse_expect(lhs, ']');
	} else if (symbol == '{') {
		rhs.set_dictionary();
		parse_dict(lhs, rhs);
		parse_expect(lhs, '}');
	} else {
		lhs.putback(symbol);
		parse_primitive(lhs, rhs);
	}

	return lhs;
}
