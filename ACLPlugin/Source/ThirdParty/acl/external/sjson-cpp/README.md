[![CLA assistant](https://cla-assistant.io/readme/badge/nfrechette/sjson-cpp)](https://cla-assistant.io/nfrechette/sjson-cpp)
[![Build status](https://ci.appveyor.com/api/projects/status/oynd3x3d9umjaruf/branch/develop?svg=true)](https://ci.appveyor.com/project/nfrechette/sjson-cpp)
[![Build Status](https://travis-ci.org/nfrechette/sjson-cpp.svg?branch=develop)](https://travis-ci.org/nfrechette/sjson-cpp)
[![Sonar Status](https://sonarcloud.io/api/project_badges/measure?project=nfrechette_sjson-cpp&metric=alert_status)](https://sonarcloud.io/dashboard?id=nfrechette_sjson-cpp)
[![GitHub (pre-)release](https://img.shields.io/github/release/nfrechette/sjson-cpp/all.svg)](https://github.com/nfrechette/sjson-cpp/releases)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/nfrechette/sjson-cpp/master/LICENSE)

# sjson-cpp

`sjson-cpp` is a C++ library to read and write [Simplified JSON](http://help.autodesk.com/view/Stingray/ENU/?guid=__stingray_help_managing_content_sjson_html) files.
It aims to be minimal, fast, and get out of the way of the programmer.

By design, the library does no memory allocations. This is in contrast to the [nflibs C parser](https://github.com/niklasfrykholm/nflibs).

Everything is **100% C++11** header based for easy and trivial integration.

This parser is intended to accept only pure SJSON, and it will fail if given a JSON file, unlike the [Autodesk JS Stingray parser](https://github.com/Autodesk/sjson).

## The SJSON format

The data format is described [here](http://help.autodesk.com/view/Stingray/ENU/?guid=__stingray_help_managing_content_sjson_html) in the Stingray documentation.

TODO: Add a reference sjson file showing the format as a form of loose specification

## Unicode support

UTF-8 support is as follow:

*  String values return a raw `StringView` into the SJSON buffer. It is the responsability of the caller to interpret it as ANSI or UTF-8.
*  String values properly support escaped unicode sequences in that they are returned raw in the `StringView`.
*  Keys do not support UTF-8, they must be ANSI.
*  The BOM is properly skipped if present

Unicode formats other than UTF-8 aren't supported.

## Supported platforms

*  Windows VS2015 x86 and x64, VS2017/2019 x86, x64, and ARM64*
*  Linux (gcc5, gcc6, gcc7, gcc8, gcc9, clang4, clang5, clang6, clang7) x86 and x64
*  OS X (Xcode 8.3, 9.4, 10.3, 11.2*) x86 and x64
*  Android (NVIDIA CodeWorks with clang5) ARMv7-A and ARM64
*  iOS (Xcode 8.3, 9.4, 10.3) ARM64

The above supported platform list is only what is tested every release but if it compiles, it should run just fine.

Notes: *VS2017* and *VS2019* compile with *ARM64* on *AppVeyor* but I have no device to test them with. Xcode 11 no longer supports x86.

## External dependencies

There are none! You don't need anything else to get started: everything is self contained.
See [here](./external) for details.

## Getting up and running

This library is **100%** headers as such you just need to include them in your own project to start using it. However, if you wish to run the unit tests you will need a few things, see below.

### Windows, Linux, and OS X for x86 and x64

1. Install *CMake 3.2* or higher (*3.14* for Visual Studio 2019, or *3.10* on OS X with *Xcode 10*), *Python 2.7 or 3*, and the proper compiler for your platform.
2. Execute `git submodule update --init` to get the files of external submodules (e.g. Catch2).
3. Generate the IDE solution with: `python make.py`  
   The solution is generated under `./build`  
   Note that if you do not have CMake in your `PATH`, you should define the `SJSON_CPP_CMAKE_HOME` environment variable to something like `C:\Program Files\CMake`.
4. Build the IDE solution with: `python make.py -build`
5. Run the unit tests with: `python make.py -unit_test`

### Windows ARM64

For *Windows on ARM64*, the steps are identical to *x86 and x64* but you will need *CMake 3.13 or higher* and you must provide the architecture on the command line: `python make.py -compiler vs2017 -cpu arm64`

### Android

For *Android*, the steps are identical to *Windows, Linux, and OS X* but you also need to install *NVIDIA CodeWorks 1R5* (or higher).

### iOS

For *iOS*, the steps are identical to the other platforms but due to code signing, you will need to perform the builds from *Xcode* manually. Note that this is only an issue if you attempt to use the tools or run the unit tests locally.

## Commit message format

This library uses the [angular.js message format](https://github.com/angular/angular.js/blob/master/DEVELOPERS.md#commits) and it is enforced with commit linting through every pull request.

## Authors

*  [Nicholas Frechette](https://github.com/nfrechette)
*  [Cody Jones](https://github.com/CodyDWJones)

## License, copyright, and code of conduct

This project uses the [MIT license](LICENSE).

Copyright (c) 2017 Nicholas Frechette, Cody Jones, and sjson-cpp contributors

Please note that this project is released with a [Contributor Code of Conduct](CODE_OF_CONDUCT.md). By participating in this project you agree to abide by its terms.
