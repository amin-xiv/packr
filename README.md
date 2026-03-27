# Packr

Simple directory archiver.

## Description

Given a directory, the tools packs it into a single ``.packr`` file, that can be unpacked using the same tool.

## Getting Started

### Dependencies

- GCC +V5
- CMake v3.3+
- Linux(since MacOS is unix-based it should *theoretically* work since this program is posix-compliant but I didn't put much care into this).

### Building

* Clone the repository
* Create a build directory
* Run `cmake --build [name of build directory]`
* Executable will be there

### Usage
 #### Packing
 ```
./packr -p [directory name]
```
 #### Unpacking
 ```
 ./packr -u [.packr file]
 ```

 #### Help
 ```
 ./packr --help
 ```

## Notice
* You will probably find some bugs because apparently I am not recreating a perfect WinRar clone or something like that.
* For now, the program skips any symlinks(yes I'm this lazy).
* It's better to run it with root permissions since if it encounters any entries that require root privilages it will simply crash :)

## License

This project is licensed under the GPL v3.0 License - see the LICENSE file for details.
