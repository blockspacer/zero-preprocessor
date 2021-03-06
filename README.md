# Zero Preprocessor

A cross-platform cross-compiler source to source translator that will allow us to play around with some new compile time language features such as Static Reflection and Meta Classes.
The AST can also be used for building static analysers.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for testing purposes.

### Prerequisites

A prerequisite for this project is Boost 1.67+
specifically Boost.Spirit x3 and Boost.Process

### Installing

For now all you need to do is git clone the repo, add it as a sub_directory
 and call the function on the target you wish to include the preprocessor for.

```
# add our preprocessor
set(preprocessor_dir "path/to/cloned/repo")

# where to build it relative to your project build dir or an absolute path
set(preprocessor_build_dir "./preprocessor_build")
add_subdirectory(${preprocessor_dir} ${preprocessor_build_dir} EXCLUDE_FROM_ALL)

# preprocess our example target
preprocess(example ${preprocessor_dir})
```
Tested on GCC 7.3, 8.3; Clang 6.0, 7.0 and MSVC 15.9

Also beware of the Clang + libstdc++ std::variant bug.

## Examples

Full examples of usages of the implemented features are located in the examples folder.

## Versioning

The project does not use versioning for now, as you should always build from the master branch.

## TODOs
Things left to be implemented

#### Meta Classes
- [x] Parsing of meta classes in code
- [x] generator -> in constexpr function
- [x] get class functions
- [x] -> variable.method()$ in {}
- [x] -> (variable)$ in {}
- [x] multiple files
- [x] -> with target function
- [x] get class variables
- [x] compiler.require
- [x] method access modifier status e.g. is_public()
- [x] constructors is_copy() is_move()
- [x] constructors/destructors status
- [x] compiler.error
- [x] compiler.require/error with f to display source location
- [x] propagate method's const and && qualifiers
- [ ] class and method templates
- [x] __metaclass_finalization
- [ ] AS & IS
- [ ] use of normal user constexpr functions inside metaclass functions
- [ ] add concepts


#### Static Reflection
- [x] reflexpr()
- [x] reflect::get_name
- [ ] reflect::get_display_name will wait for C++20
- [x] reflect::get_type
- [ ] reflect::get_reflected_type
- [x] reflect::is_[enum, class, struct, union]
- [x] reflect::get_public_data_members
- [x] reflect::get_size and reflect::get_element
- [x] reflect::get_pointer (for data members)
- [x] reflect::get_data_members
- [x] reflect::get_public_base_classes
- [x] reflect::get_base_classes
- [ ] reflect::get_accessible_[data_members, base_classes] based on context
- [ ] reflect::is_final & reflect::is_virtual
- [x] enum operations
- [x] reflect::get_constant for enumerator values
- [ ] reflexpr() for variables and namespaces
- [ ] reflect::is_inline for namespaces
- [ ] add concepts

#### Parser:
- [x] variables
- [x] expressions
- [x] functions
- [x] struct/class
- [x] methods and data members
- [x] constructors/destructors
- [x] virtual/override methods
- [ ] C style arrays
- [ ] index operator
- [ ] new/delete malloc/free
- [x] for loop and ranged base for
- [x] while loop and else
- [x] type templates
- [x] non-type templates
- [ ] variadic templates
- [x] Inheritance
- [ ] virtual inheritance
- [x] if constexpr
- [x] namespaces
- [ ] C++17 namespaces n1::n2
- [x] enum
- [ ] union
- [x] constexpr functions
- [x] lambdas
- [ ] C++17 structured binding declaration
- [ ] C++14 functions with -> return type


## Contributing

There are TODO comments throughout the project. Before starting to fix one please open an issue referencing the TODO in code.

## License

This project is licensed under the Apache License 2.0 License - see the [LICENSE](LICENSE) file for details
