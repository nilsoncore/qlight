#!/bin/sh

# Linux build script file.
# Usage:
# "./build" or "./build all" - Execute both Debug and Release builds;
# "./build debug" - Execute Debug build;
# "./build release" - Execute Release build.

build_debug=0
build_release=0
arg1=$1

# Switch between build modes.
if [ -z "$arg1" ]; then
	build_debug=1
	build_release=1
	echo "Build option argument is not provided, executing both Debug and Release builds."
elif [ $arg1 = "debug" ]; then
	build_debug=1
	echo "Build option argument is set to \"$arg1\", executing Debug build."
elif [ $arg1 = "release" ]; then
	build_release=1
	echo "Build option argument is set to \"$arg1\", executing Release build."
elif [ $arg1 = "all" ]; then
	build_debug=1
	build_release=1
	echo "Build option argument is set to \"$arg1\", executing both Debug and Release builds."
else
	echo "Build option argument \"$arg1\" is not valid. Please, use either \"debug\", \"release\", \"all\", or leave it empty for the same effect."
	exit 1
fi

# Build in Debug mode.
if [ $build_debug -eq 1 ]; then

	echo ""
	echo "--- Debug build ---"
	echo ""

	msbuild.exe "qlight.sln" -nologo -property:Configuration=Debug -property:Platform=x64 -target:Build
	build_debug_result=$?

	echo ""
	if [ $build_debug_result -ne 0 ]; then
		echo "Failed to compile Debug build!"
		echo ""
		exit $build_debug_result
	fi

fi

# Build in Release mode.
if [ $build_release -eq 1 ]; then

	echo ""
	echo "--- Release build ---"
	echo ""

	msbuild.exe "qlight.sln" -nologo -property:Configuration=Release -property:Platform=x64 -target:Build
	build_release_result=$?

	echo ""
	if [ $build_release_result -ne 0 ]; then
		echo "Failed to compile Release build!"
		echo ""
		exit $build_release_result
	fi

fi

echo ""
echo "All builds completed."
echo ""