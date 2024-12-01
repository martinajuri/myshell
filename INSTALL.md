# Installation Instructions

## Define the Project Root
export PROJECT_ROOT=/path/to/project/root

## Install Conan & CMake
To install Conan, run the following commands:
    ```
    sudo apt-get install python3-pip
    pip3 install conan
    ```
To install CMake, run the following commands:

    ```
    sudo apt-get install cmake
    ```

## Adding the Submodule
To add the submodule, run the following commands:

```git submodule sync
git submodule update --init --recursive
```
## Building and Compiling
To build and compile the program, execute the following commands:

```
source venv/bin/activate
conan profile detect --force 
rm -rf build
mkdir build
conan install . --output-folder=build --build=missing 
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=./build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
make
```

## Coverage
To install coverage tools and run coverage checks, use the following commands:

```
sudo apt-get install lcov
ctest
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/' 'test/' --output-file coverage_filtered.info
```
