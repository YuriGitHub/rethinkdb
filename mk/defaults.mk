# Copyright 2010-2013 RethinkDB, all rights reserved.

# This file contains the default values of all the variables that can affect the build that
# are not set by the configure script. All these variables should be listed and documented here.
# mk/check-env.mk generates warnings when a user sets a variable that is not defined in here.

# Place local and temporary changes to these variables in a file called custom.mk at the
# root of the source tree instead of modifying this file.

# Configuration file generated by ./configure
CONFIG ?= $(TOP)/config.mk

# Default goal
DEFAULT_GOAL ?= all

# If set to 1, make will not recompile when one of the makefiles changes
IGNORE_MAKEFILE_CHANGES ?= 0

# If set to 1, do not use -Werror
ALLOW_WARNINGS ?= 1

# Show the number of targets left to build
SHOW_COUNTDOWN ?= 1

# If set to 1, make will show every command that it executes
VERBOSE ?= 0

# set to 1 to link all libraries statically
STATIC ?= 0

# Package name without modfiers
VANILLA_PACKAGE_NAME ?= rethinkdb

# Name of the rethinkdb executable
SERVER_EXEC_NAME ?= rethinkdb

# Set SYMBOLS to 1 to enable symbols, even in release mode
SYMBOLS ?= 1

# Put the symbols in a separate file
SPLIT_SYMBOLS ?= 0

# Add numeric indices to json objects in the json adapter
JSON_SHORTCUTS ?= 0

# Set to 0 for release mode or 1 for debug mode
DEBUG ?= 0

# Build unit tests
UNIT_TESTS ?= $(DEBUG)

# Set to 1 to enable valgrind awareness
VALGRIND ?= 0

# Build directory (empty for default)
BUILD_DIR ?=

# Destination directory when installing. Defaults to /
DESTDIR ?=

# Check for unknown variables in custom.mk
MAKE_VARIABLE_CHECK ?= 1

# Do not allow unknown variables in custom.mk
STRICT_MAKE_VARIABLE_CHECK ?= 0

# Enable code coverage
COVERAGE ?= 0

# Strip the executable when installing
STRIP_ON_INSTALL ?= 0

# If not empty, $(PVERSION) is used as the rethinkdb version number
# for example, PVERSION ?= 1.3.2
PVERSION ?=

# Include the rethinkdb version number in the executable name and package name
NAMEVERSIONED ?= 0

# When packaging, build a package for this ubuntu or debian release name
UBUNTU_RELEASE ?=
DEB_RELEASE ?=

# What tests to run
TEST ?= default

# How to run the tests (arguments for ./scripts/run-tests.sh)
RUN_TEST_ARGS ?=

# For each triggered rule, show one of the dependencies that caused it to be run
SHOW_BUILD_REASON ?= 0

# Enable backtraces for RQL errors
RQL_ERROR_BT ?= 0

# Always collect full perfmon stats. This makes RethinkDB slower
# but can be useful for analyzing performance issues.
FULL_PERFMON ?= 0

# Turn on the coroutine profiler
CORO_PROFILING ?= 0

# Sign the DSC file
SIGN_PACKAGE ?= 1

# When rebuilding the package for an already published version of RethinkDB, this
# variable should be incremented.
PACKAGE_BUILD_NUMBER ?= 0

# Implement coroutines on top of (POSIX) threads
THREADED_COROUTINES ?= 0

# Require MacOS package to be signed with a developer certificate
REQUIRE_SIGNED ?= 0
OSX_SIGNATURE_NAME ?= Hexagram 49, Inc. (99WDWQ7WDJ)

# Adds default configure flags to the package being built
DIST_CONFIGURE_DEFAULT ?=

# Minify the webui source code with "uglify".
UGLIFY ?= 1

# Sets the `-fno-omit-frame-pointer` compiler flag. Important for some profiling tools
# such as oprofile.
NO_OMIT_FRAME_POINTER ?= 0

# Check the hash of fetched archives
VERIFY_FETCH_HASH=1

# TODO: Document these variables
STATIC_LIBGCC ?= 0
DISABLE_BREAKPOINTS ?= 0
BUILD_PORTABLE ?= 0
LEGACY_LINUX ?= 0
LEGACY_GCC ?= 0
RT_FORCE_NATIVE ?= 0
RT_COPY_NATIVE ?= 0
RT_REDUCE_NATIVE ?= 0
KEEP_INLINE ?= 0
NO_EVENTFD ?= 0
NO_EPOLL ?= 0
UNIT_TEST_FILTER ?= *
PACKAGE_FOR_SUSE_10 ?= 0
NO_COMPILE_JS ?= 0
