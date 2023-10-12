#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>
using namespace std;

#ifdef NDEBUG
#define debug(a)
#else
#define debug(a) cout << __FILE__ << ':' << __LINE__ << ": " << __func__ << ": " << #a << ": " << (a) << '\n'
#endif

string file;
string text;

void readText() {
	ifstream is(file, ios::binary);
	text = {istreambuf_iterator<char>(is), istreambuf_iterator<char>()};

	// make sure input ends with a newline, to simplify parser code
	if (text.empty() || text.back() != '\n')
		text += '\n';
}

bool isid(unsigned char c) {
	return isalnum(c) || c == '_';
}

[[noreturn]] void err(const char* t, string msg) {
	int line = 1;
	for (auto s = text.data(); s != t; ++s)
		if (*s == '\n')
			++line;
	throw runtime_error(file + ':' + to_string(line) + ": " + msg);
}

struct Tok {
	const char* first;
	const char* last;

	Tok(const char* first, const char* last): first(first), last(last) {
	}

	virtual bool eq(const char*) {
		return 0;
	}

	virtual string word() {
		err(first, "expected word");
	}
};

struct Comment: Tok {
	Comment(const char* first, const char* last): Tok(first, last) {
	}
};

struct Word: Tok {
	Word(const char* first, const char* last): Tok(first, last) {
	}

	virtual bool eq(const char* t) {
		for (auto s = first; s != last; ++s)
			if (tolower((unsigned char)*s) != *t++)
				return 0;
		return !*t;
	}

	virtual string word() {
		return string(first, last);
	}
};

vector<Tok*> toks;

void lex() {
	toks.clear();
	auto s = text.data();
	while (*s) {
		auto t = s;
		Tok* tok;
		switch (*s) {
		case '-':
			if (s[1] == '-') {
				while (t[0] == '-' && t[1] == '-') {
					t = strchr(t, '\n');
					while (isspace((unsigned char)*t))
						++t;
				}
				tok = new Comment(s, t);
			} else {
				++t;
				tok = new Tok(s, t);
			}
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
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
		case 'z':
			do
				++t;
			while (isid(*t));
			tok = new Word(s, t);
			break;
		case '\'':
			++t;
			while (*t != '\'') {
				switch (*t) {
				case '\\':
					t += 2;
					continue;
				case '\n':
					err(s, "unclosed quote");
				}
				++t;
			}
			++t;
			tok = new Tok(s, t);
			break;
		default:
			++t;
			tok = new Tok(s, t);
		}
		toks.push_back(tok);
		s = t;
	}
	toks.push_back(new Tok(s, s));
}

void expect(size_t i, char c) {
	if (toks[i]->first[0] != c)
		err(toks[i]->first, string("expected '") + c + '\'');
}

struct Table {
	string name;
	vector<Table*> links;

	Table(string name): name(name) {
	}
};

vector<Table*> tables;

void parse() {
	for (size_t i = 0; i != toks.size();) {
		if (toks[i]->eq("create") && toks[i + 1]->eq("table")) {
			auto table = new Table(toks[i+2]->word());
			i += 3;
			expect(i++, '(');
			size_t depth = 1;
			for (auto j = i + 4; depth; ++j)
				switch (toks[j]->first[0]) {
				case '(':
					++depth;
					break;
				case ')':
					--depth;
					break;
				}
				tables.push_back(table);
				continue;
		}
		 ++i;
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
			lex();
			parse();
		}
		return 0;
	} catch (exception& e) {
		cerr << e.what() << '\n';
		return 1;
	}
}
