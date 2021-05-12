## Is metastate feasible with OO ?

We think it is, please see the slightly longer example bellow.  Code is also avialble on [Godbolt](https://godbolt.org/z/dPzvrx8Tv):
```cpp
// (c) 2021 by dbj"dbj.org CC BY SA 4.0
#undef NDEBUG
#define DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NDEBUG  // aka release  mode
#define debug(M, ...)
#else
#define debug(M, ...)                                                   \
  fprintf(stderr, "DEBUG %s (in function '%s'):%d:  " M "\n", __FILE__, \
          __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
// show the expression and the value it yields to
#define DBG(FMT_, X_) debug("%s = " FMT_, #X_, (X_))

// ;)
using string = const char *;

// (c) 2019 - 2021 by dbj@dbj.org
namespace std {
template <typename T, typename S>
struct [[nodiscard]] valstat {
  // both types must be able to
  // simply and resiliently
  // exhibit state of occupancy
  // "empty" or "has value"
  using value_type = T;
  using status_type = S;

  // metastate is captured by AND-ing
  // state of occupancy of these two
  value_type value;
  status_type status;
};
}  // namespace std
// the metastate constructor mechanism
struct person final {
  // nested valstat type
  using valstat = std::valstat<person, const string>;

  string name;

  // rule 2: for constructor to be callable
  // it must return by value instance of one
  // of the the nested valstat types
  constexpr person() noexcept : name("Default") {}

  constexpr person(string new_name_) noexcept : name(new_name_) {}

  // ad-hoc "rule"
  // abstraction carried by valstat must have
  // an method named empty()
  // returns true if instance is "empty" by the
  // logic of the abstraction
  bool empty() const noexcept { return strlen(name) < 1; }
  bool empty() noexcept { return strlen(name) < 1; }

};  // person

    /// --------------------------------------------------------------------
    /// valstat returning factory method
constexpr inline person::valstat make_person(string const &name_) noexcept {
  return {person(name_), "Info"};
}

int main(void) {
  // structured binding and constexpr do not mix, yet?
  if (auto [person_a, status] = make_person("Stack"); !person_a.empty()) {
    DBG("%s", person_a.name);
  }
  /*
  can not decompose a pointer to valstat, of course
  auto [ person_a , status ] = new person::valstat{ person("B"), {} };
  */
  using valstat_person_pointer = std::valstat<person *, const string>;
  /*
  but can use it as a value field of the valstat
  */
  if (auto [person_a, status] = valstat_person_pointer{new person("Heap"), {}};
      // remember: person is a pointer here
      !person_a->empty()) {
    DBG("%s", person_a->name);
    delete person_a;
  } else {
    DBG("%s", "Heap allocation failed");
  }
}

```
- Constructor can not return valstat (yet?), ditto we used the factory function. 
     - Notice in that function we have achieved both compile time and pure function. Also enabled thanks to using valstat to return the metastate. 
     - Note we could use that at compile time but not as structured return due to the C++ rules.
- Obviously OO practitioners who are also metastate adopters might have other very different, but equally valid designs,

