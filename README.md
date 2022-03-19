**fq** is an open-source formatted input library providing a fast alternative to C stdio and C++ iostreams.

If you like this project, please consider donating to me.

# Features
- Simple format syntas compatitable with {fmt}
- Extensibility: support for user-defined types
- Small code size both in terms of source code with the minimum configuration consisting of just three files, core.h, format.h and format-inl.h, and compiled code; see Compile time and code bloat
- Ease of use: small self-contained code base, no external dependencies, permissive MIT license
- Portability with consistent output across platforms and support for older compilers
- Clean warning-free codebase even on high warning levels such as -Wall -Wextra -pedantic
- Locale-independence by default

# Examples
**Basic**
```C++
./tool_matcher --format '{name}|{age}' --source 'Alice|18'
// output
// name: Alice
// age: 18
```

**with type**
```C++
./tool_matcher --format '{name:str}|{age:int}' --source 'Alice|18'
// output
// name str: Alice
// age int: 18
```

**Recursion**
```C++
./tool_matcher --format '{name:str}:{Raw({age:int})}' --source 'Alice|18'
// output
// name str: Alice
// age int: 18
```
this is useless, but the inner function can be defined by user.

# Motivation
So why yet another scanf library?

There are plenty of parser for doing this task, from standard ones like the scanf family of function and iostreams to regex and boost xpressive libraries. The reason for creating a new library is that every existing solution that I found either had serious issues or didn't provide the features I needed.

# Last But NOT Least
Since printf has evolved an elegant formatting library fmt, why not scanf also have its own evolved library?
