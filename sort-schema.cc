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

enum {
	// SORT
	t_comment,
	t_etc,
	t_space,
	t_word,
};

struct Tok {
	int tag;
	int i, j;

	Tok(int tag, int i, int j): tag(tag), i(i), j(j) {
	}
};

vector<Tok> toks;

void lex() {
	toks.clear();
	auto s = text.data();
	int i = 0;
	while (s[i]) {
		auto j = i;
		switch (s[i]) {
		case ' ':
		case '\n':
		case '\r':
		case '\t':
			++j;
			toks.emplace_back(t_space, i, j);
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
			toks.emplace_back(t_word, i, j);
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
			toks.emplace_back(t_etc, i, j);
			break;
		}
		i = j;
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
		}
		return 0;
	} catch (exception& e) {
		cerr << e.what() << '\n';
		return 1;
	}
}
