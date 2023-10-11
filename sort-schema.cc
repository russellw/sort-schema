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
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

#ifdef NDEBUG
#define debug(a)
#else
#define debug(a)                                                               \
  cout << __FILE__ << ':' << __LINE__ << ": " << __func__ << ": " << #a        \
       << ": " << (a) << '\n'
#endif

template <class T>
void topologicalSortRecur(const vector<T> &v, vector<T> &o,
                          unordered_set<T> &visited, T a) {
  if (!visited.insert(a).second)
    return;
  for (auto b : a->links)
    topologicalSortRecur(v, o, visited, b);
  o.push_back(a);
}

template <class T> void topologicalSort(vector<T> &v) {
  unordered_set<T> visited;
  vector<T> o;
  for (auto a : v)
    topologicalSortRecur(v, o, visited, a);
  v = o;
}

int main(int argc, char **argv) {
  try {
    bool inplace = 0;
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
    return 0;
  } catch (exception &e) {
    cerr << e.what() << '\n';
    return 1;
  }
}
