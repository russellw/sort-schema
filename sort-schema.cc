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

[[noreturn]] void err(int i, string msg) {
	auto s = text.data();
	int line = 1;
	for (int j = 0; j < i; ++j)
		if (s[j] == '\n')
			++line;
	throw runtime_error(file + ':' + to_string(line) + ": " + msg);
}

struct Tok {
	int first, last;

	Tok(int first, int last): first(first), last(last) {
	}

	virtual bool eq(const char*) {
		return 0;
	}
};

struct Comment: Tok {
	Comment(int first, int last): Tok(first, last) {
	}
};

struct Word: Tok {
	Word(int first, int last): Tok(first, last) {
	}

	virtual bool eq(const char* t) {
		auto s = text.data();
		auto i = first;
		for (; i < last; ++i)
			if (tolower((unsigned char)(s[i])) != t[i])
				return 0;
		return !t[i];
	}
};

vector<Tok*> toks;

void lex() {
	toks.clear();
	auto s = text.data();
	for (int i = 0; s[i];) {
		auto j = i;
		Tok* tok;
		switch (s[i]) {
		case '-':
			if (s[1] == '-') {
				auto t = s + i;
				while (t[0] == '-' && t[1] == '-') {
					t = strchr(t, '\n');
					while (isspace((unsigned char)*t))
						++t;
				}
				j = t - s;
				tok = new Comment(i, j);
			} else {
				++j;
				tok = new Tok(i, j);
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
				++j;
			while (isid(s[j]));
			tok = new Word(i, j);
			break;
		case '\'':
			++j;
			while (s[j] != '\'') {
				switch (s[j]) {
				case '\\':
					j += 2;
					continue;
				case '\n':
					err(i, "unclosed quote");
				}
				++j;
			}
			++j;
			tok = new Tok(i, j);
			break;
		default:
			++j;
			tok = new Tok(i, j);
		}
		toks.push_back(tok);
		i = j;
	}
	toks.push_back(new Tok(0, 0));
}

struct Table {
	string name;
	vector<Table*> links;
};

vector<Table*> tables;

void parse() {
	for (int i = 0; i < toks.size(); ++i) {
		if (toks[i]->eq("create") && toks[i + 1]->eq("table")) {
		}
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
