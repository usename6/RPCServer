# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/reactor_demo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/reactor_demo

# Include any dependencies generated for this target.
include CMakeFiles/HttpRpcServer.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/HttpRpcServer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/HttpRpcServer.dir/flags.make

CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o: CMakeFiles/HttpRpcServer.dir/flags.make
CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o: src/RpcServer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/reactor_demo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o -c /home/reactor_demo/src/RpcServer.cpp

CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/reactor_demo/src/RpcServer.cpp > CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.i

CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/reactor_demo/src/RpcServer.cpp -o CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.s

CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o.requires:

.PHONY : CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o.requires

CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o.provides: CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o.requires
	$(MAKE) -f CMakeFiles/HttpRpcServer.dir/build.make CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o.provides.build
.PHONY : CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o.provides

CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o.provides.build: CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o


# Object files for target HttpRpcServer
HttpRpcServer_OBJECTS = \
"CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o"

# External object files for target HttpRpcServer
HttpRpcServer_EXTERNAL_OBJECTS =

bin/HttpRpcServer: CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o
bin/HttpRpcServer: CMakeFiles/HttpRpcServer.dir/build.make
bin/HttpRpcServer: lib/libHTTPLIB.so
bin/HttpRpcServer: CMakeFiles/HttpRpcServer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/reactor_demo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable bin/HttpRpcServer"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/HttpRpcServer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/HttpRpcServer.dir/build: bin/HttpRpcServer

.PHONY : CMakeFiles/HttpRpcServer.dir/build

CMakeFiles/HttpRpcServer.dir/requires: CMakeFiles/HttpRpcServer.dir/src/RpcServer.cpp.o.requires

.PHONY : CMakeFiles/HttpRpcServer.dir/requires

CMakeFiles/HttpRpcServer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/HttpRpcServer.dir/cmake_clean.cmake
.PHONY : CMakeFiles/HttpRpcServer.dir/clean

CMakeFiles/HttpRpcServer.dir/depend:
	cd /home/reactor_demo && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/reactor_demo /home/reactor_demo /home/reactor_demo /home/reactor_demo /home/reactor_demo/CMakeFiles/HttpRpcServer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/HttpRpcServer.dir/depend

