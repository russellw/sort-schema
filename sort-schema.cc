#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
using namespace std;

#ifdef NDEBUG
#define debug(a)
#else
#define debug(a) cout << __FILE__ << ':' << __LINE__ << ": " << __func__ << ": " << #a << ": " << (a) << '\n'
#endif

string file;
string text;

// SORT
bool isid(unsigned char c) {
	return isalnum(c) || c == '_';
}

void readText() {
	ifstream is(file, ios::binary);
	text = {istreambuf_iterator<char>(is), istreambuf_iterator<char>()};

	// make sure input ends with a newline, to simplify parser code
	if (text.empty() || text.back() != '\n')
		text += '\n';
}

// tokenizer
enum {
	k_quote = 0x100,
	k_word,
};

// SORT
const char* commentFirst;
const char* commentLast;
const char* first;
const char* src;
int tok;
//

[[noreturn]] void err(string msg) {
	size_t line = 1;
	for (auto s = text.data(); s != src; ++s)
		if (*s == '\n')
			++line;
	throw runtime_error(file + ':' + to_string(line) + ": " + msg);
}

[[noreturn]] void err(const char* s, string msg) {
	src = s;
	err(msg);
}

void lex() {
	for (;;) {
		first = src;
		switch (*src) {
		case ' ':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
			++src;
			continue;
		case '"':
		case '\'': {
			auto q = *src;
			auto s = src + 1;
			while (*s != q) {
				switch (*s) {
				case '\\':
					s += 2;
					continue;
				case '\n':
					err("unclosed quote");
				}
				++s;
			}
			src = s + 1;
			tok = k_quote;
			return;
		}
		case '-': {
			auto s = src;
			commentFirst = s;
			if (s[1] == '-') {
				// we need to do more than just skip over comments
				// but also track the full extent of each comment block
				do {
					s = strchr(s, '\n');
					do
						++s;
					while (isspace((unsigned char)*s));
				} while (s[0] == '-' && s[1] == '-');
				commentLast = s;
				src = s;
				continue;
			}
			break;
		}
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': {
			auto s = src;
			do
				++s;
			while (isid(*s));
			src = s;
			tok = k_word;
			return;
		}
		case '[': {
			auto s = src + 1;
			while (*s != ']') {
				if (*s == '\n')
					err("unclosed '['");
				++s;
			}
			src = s + 1;
			tok = k_quote;
			return;
		}
		case 0:
			tok = 0;
			return;
		}
		tok = *src++;
		return;
	}
}

void initLex() {
	commentFirst = 0;
	commentLast = 0;
	src = text.data();
	lex();
}

// parser
bool eat(int k) {
	if (tok != k)
		return 0;
	lex();
	return 1;
}

bool eat(const char* t) {
	if (tok != k_word)
		return 0;
	auto s = first;
	auto n = strlen(t);
	if (src - s != n)
		return 0;
	for (size_t i = 0; i != n; ++i)
		if (tolower((unsigned char)s[i]) != t[i])
			return 0;
	lex();
	return 1;
}

void expect(char k) {
	if (!eat(k))
		err(string("expected '") + k + '\'');
}

string name() {
	string s;
	switch (tok) {
	case k_quote:
		s.assign(first + 1, src - 1);
		break;
	case k_word:
		s.assign(first, src);
		break;
	default:
		err("expected name");
	}
	lex();
	return s;
}

struct Table {
	const char* first;
	const char* last;
	string name;
	vector<pair<const char*, string>> refs;
	vector<Table*> links;

	Table(const char* first, string name): first(first), name(name) {
	}
};

vector<Table*> tables;

void parse() {
	while (tok) {
		// record any comment block before CREATE
		// because, though unlikely, it is possible there could be another one between words
		auto commentFirst = ::commentFirst;
		auto commentLast = ::commentLast;

		// and the location of the CREATE keyword
		auto createFirst = ::first;

		// if there is one
		if (!(eat("create") && eat("table"))) {
			lex();
			continue;
		}

		// if there was a comment block immediately before this table
		// consider it part of the table text
		auto table = new Table(commentLast == createFirst ? commentFirst : createFirst, name());

		expect('(');
		size_t depth = 1;
		while (depth) {
			switch (tok) {
			case '(':
				++depth;
				break;
			case ')':
				--depth;
				break;
			case 0:
				err(createFirst, "unclosed CREATE TABLE");
			case k_word:
				if (eat("references")) {
					auto s = first;
					table->refs.emplace_back(s, name());
					continue;
				}
				break;
			}
			lex();
		}
		table->last = src;
		expect(';');

		tables.push_back(table);
		continue;
	}
}

// resolve names to pointers to tables
void link() {
	unordered_map<string, Table*> m;
	for (auto table: tables) {
		auto& t = m[table->name];
		if (t)
			err(table->first, table->name + ": duplicate name");
		t = table;
	}

	for (auto table: tables)
		for (auto& r: table->refs) {
			auto t = m[r.second];
			if (!t)
				err(r.first, r.second + ": not found");
			table->links.push_back(t);
		}
}

template <class T> void topologicalSortRecur(const vector<T>& v, vector<T>& o, unordered_set<T>& visited, T a) {
	if (!visited.insert(a).second)
		return;
	for (auto b: a->links)
		topologicalSortRecur(v, o, visited, b);
	o.push_back(a);
}

template <class T> void topologicalSort(vector<T>& v) {
	unordered_set<T> visited;
	vector<T> o;
	for (auto a: v)
		topologicalSortRecur(v, o, visited, a);
	v = o;
}

int main(int argc, char** argv) {
	try {
		bool inplace = 0;
		vector<string> files;
		for (int i = 1; i < argc; ++i) {
			auto s = argv[i];
			if (*s == '-') {
				while (*s == '-')
					++s;
				switch (*s) {
				case 'V':
				case 'v':
					cout << "sort-schema version 0\n";
					return 0;
				case 'h':
					cout << "-h  Show help\n";
					cout << "-V  Show version\n";
					cout << "-i  Edit files in place\n";
					return 0;
				case 'i':
					inplace = 1;
					continue;
				}
				throw runtime_error(string(argv[i]) + ": unknown option");
			}
			files.push_back(s);
		}

		for (auto& file0: files) {
			file = file0;
			readText();
			initLex();
			parse();
			if (tables.empty())
				throw runtime_error(file + ": no tables found");
			link();

			auto sorted = tables;
			sort(sorted.begin(), sorted.end(), [](const Table* a, const Table* b) { return a->name < b->name; });
			topologicalSort(sorted);

			tables.push_back(new Table(text.data() + text.size(), ""));
			string o((const char*)text.data(), tables[0]->first);
			for (size_t i = 0; i != sorted.size(); ++i) {
				o.append(sorted[i]->first, sorted[i]->last);
				o.append(tables[i]->last, tables[i + 1]->first);
			}

			if (inplace) {
				if (o == text)
					continue;
				ofstream os(file, ios::binary);
				os << o;
				continue;
			}
			cout << o;
		}
		return 0;
	} catch (exception& e) {
		cerr << e.what() << '\n';
		return 1;
	}
}
