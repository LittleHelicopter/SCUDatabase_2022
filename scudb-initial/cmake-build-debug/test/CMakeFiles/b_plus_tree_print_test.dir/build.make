# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug

# Include any dependencies generated for this target.
include test/CMakeFiles/b_plus_tree_print_test.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/b_plus_tree_print_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/b_plus_tree_print_test.dir/flags.make

test/CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.o: test/CMakeFiles/b_plus_tree_print_test.dir/flags.make
test/CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.o: ../test/index/b_plus_tree_print_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.o"
	cd /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.o -c /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/test/index/b_plus_tree_print_test.cpp

test/CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.i"
	cd /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/test/index/b_plus_tree_print_test.cpp > CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.i

test/CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.s"
	cd /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/test/index/b_plus_tree_print_test.cpp -o CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.s

# Object files for target b_plus_tree_print_test
b_plus_tree_print_test_OBJECTS = \
"CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.o"

# External object files for target b_plus_tree_print_test
b_plus_tree_print_test_EXTERNAL_OBJECTS =

test/b_plus_tree_print_test: test/CMakeFiles/b_plus_tree_print_test.dir/index/b_plus_tree_print_test.cpp.o
test/b_plus_tree_print_test: test/CMakeFiles/b_plus_tree_print_test.dir/build.make
test/b_plus_tree_print_test: lib/libvtable.so
test/b_plus_tree_print_test: lib/libsqlite3.so
test/b_plus_tree_print_test: lib/libgtest.so
test/b_plus_tree_print_test: test/CMakeFiles/b_plus_tree_print_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable b_plus_tree_print_test"
	cd /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/b_plus_tree_print_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/b_plus_tree_print_test.dir/build: test/b_plus_tree_print_test

.PHONY : test/CMakeFiles/b_plus_tree_print_test.dir/build

test/CMakeFiles/b_plus_tree_print_test.dir/clean:
	cd /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug/test && $(CMAKE_COMMAND) -P CMakeFiles/b_plus_tree_print_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/b_plus_tree_print_test.dir/clean

test/CMakeFiles/b_plus_tree_print_test.dir/depend:
	cd /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/test /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug/test /mnt/e/learn/数据库/数据库_作业/SCUDatabase-2022/Homework2/scudb-initial/cmake-build-debug/test/CMakeFiles/b_plus_tree_print_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/b_plus_tree_print_test.dir/depend

