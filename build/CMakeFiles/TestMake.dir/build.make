# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = C:\msys64\mingw64\bin\cmake.exe

# The command to remove a file.
RM = C:\msys64\mingw64\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\ebkc\Documents\testmake

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\ebkc\Documents\testmake\build

# Include any dependencies generated for this target.
include CMakeFiles/TestMake.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/TestMake.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/TestMake.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/TestMake.dir/flags.make

CMakeFiles/TestMake.dir/main.cpp.obj: CMakeFiles/TestMake.dir/flags.make
CMakeFiles/TestMake.dir/main.cpp.obj: CMakeFiles/TestMake.dir/includes_CXX.rsp
CMakeFiles/TestMake.dir/main.cpp.obj: ../main.cpp
CMakeFiles/TestMake.dir/main.cpp.obj: CMakeFiles/TestMake.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\ebkc\Documents\testmake\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/TestMake.dir/main.cpp.obj"
	C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/TestMake.dir/main.cpp.obj -MF CMakeFiles\TestMake.dir\main.cpp.obj.d -o CMakeFiles\TestMake.dir\main.cpp.obj -c C:\Users\ebkc\Documents\testmake\main.cpp

CMakeFiles/TestMake.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TestMake.dir/main.cpp.i"
	C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ebkc\Documents\testmake\main.cpp > CMakeFiles\TestMake.dir\main.cpp.i

CMakeFiles/TestMake.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TestMake.dir/main.cpp.s"
	C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ebkc\Documents\testmake\main.cpp -o CMakeFiles\TestMake.dir\main.cpp.s

CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.obj: CMakeFiles/TestMake.dir/flags.make
CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.obj: CMakeFiles/TestMake.dir/includes_CXX.rsp
CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.obj: ../include/vendor/RectangleBinPack/GuillotineBinPack.cpp
CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.obj: CMakeFiles/TestMake.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\ebkc\Documents\testmake\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.obj"
	C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.obj -MF CMakeFiles\TestMake.dir\include\vendor\RectangleBinPack\GuillotineBinPack.cpp.obj.d -o CMakeFiles\TestMake.dir\include\vendor\RectangleBinPack\GuillotineBinPack.cpp.obj -c C:\Users\ebkc\Documents\testmake\include\vendor\RectangleBinPack\GuillotineBinPack.cpp

CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.i"
	C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ebkc\Documents\testmake\include\vendor\RectangleBinPack\GuillotineBinPack.cpp > CMakeFiles\TestMake.dir\include\vendor\RectangleBinPack\GuillotineBinPack.cpp.i

CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.s"
	C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ebkc\Documents\testmake\include\vendor\RectangleBinPack\GuillotineBinPack.cpp -o CMakeFiles\TestMake.dir\include\vendor\RectangleBinPack\GuillotineBinPack.cpp.s

CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.obj: CMakeFiles/TestMake.dir/flags.make
CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.obj: CMakeFiles/TestMake.dir/includes_CXX.rsp
CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.obj: ../include/vendor/RectangleBinPack/Rect.cpp
CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.obj: CMakeFiles/TestMake.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\ebkc\Documents\testmake\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.obj"
	C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.obj -MF CMakeFiles\TestMake.dir\include\vendor\RectangleBinPack\Rect.cpp.obj.d -o CMakeFiles\TestMake.dir\include\vendor\RectangleBinPack\Rect.cpp.obj -c C:\Users\ebkc\Documents\testmake\include\vendor\RectangleBinPack\Rect.cpp

CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.i"
	C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ebkc\Documents\testmake\include\vendor\RectangleBinPack\Rect.cpp > CMakeFiles\TestMake.dir\include\vendor\RectangleBinPack\Rect.cpp.i

CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.s"
	C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ebkc\Documents\testmake\include\vendor\RectangleBinPack\Rect.cpp -o CMakeFiles\TestMake.dir\include\vendor\RectangleBinPack\Rect.cpp.s

# Object files for target TestMake
TestMake_OBJECTS = \
"CMakeFiles/TestMake.dir/main.cpp.obj" \
"CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.obj" \
"CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.obj"

# External object files for target TestMake
TestMake_EXTERNAL_OBJECTS =

TestMake.exe: CMakeFiles/TestMake.dir/main.cpp.obj
TestMake.exe: CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/GuillotineBinPack.cpp.obj
TestMake.exe: CMakeFiles/TestMake.dir/include/vendor/RectangleBinPack/Rect.cpp.obj
TestMake.exe: CMakeFiles/TestMake.dir/build.make
TestMake.exe: CMakeFiles/TestMake.dir/linklibs.rsp
TestMake.exe: CMakeFiles/TestMake.dir/objects1.rsp
TestMake.exe: CMakeFiles/TestMake.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\ebkc\Documents\testmake\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable TestMake.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\TestMake.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/TestMake.dir/build: TestMake.exe
.PHONY : CMakeFiles/TestMake.dir/build

CMakeFiles/TestMake.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\TestMake.dir\cmake_clean.cmake
.PHONY : CMakeFiles/TestMake.dir/clean

CMakeFiles/TestMake.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\ebkc\Documents\testmake C:\Users\ebkc\Documents\testmake C:\Users\ebkc\Documents\testmake\build C:\Users\ebkc\Documents\testmake\build C:\Users\ebkc\Documents\testmake\build\CMakeFiles\TestMake.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/TestMake.dir/depend

