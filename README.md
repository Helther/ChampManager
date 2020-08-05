# Champoinship Manager
<img align=right src="rsc/better_icon.png" width='256' alt='Icon'>


## Getting Started

## Features

## Usage



## Dependencies
1. A C++ compiler that supports C++17.
The following compilers should work:

  * [gcc 7+](https://gcc.gnu.org/)

  * [clang 6+](https://clang.llvm.org/)

2. [CMake 3.12+](https://cmake.org/)

3. [Qt 5.14.1+](https://www.qt.io/)

### Build
You can build main target via CMake or using .pro file
```
 $ cmake --build --target ChampManager
```

## Testing
For testing was used CMake's CTest. [CTest docs](https://cmake.org/cmake/help/latest/manual/ctest.1.html) So far there is one unit test, for the file parser.
### Building the tests
Ctests are build via CMake by specifying test target.
```
 $ cmake --build --target test_parser
```

### Running the tests
To run built tests
```
 $ ctest -C Debug
```

## Versioning

0.1 - added file parsing capabilities

0.2 - added and integrated gui for results viewing

### Future plans

 * add user profile with championship settings and statistics
 * add gui for viewing/add/editing championship participants
 * add championship progression after every new race and season with periodic udpates to teams and drivers

## License

This project is licensed under the GNU GPLv2 License - see the [LICENSE.md](LICENSE.md) file for details.
Some third party files are subjective to their respective license.
