/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include <argvHelper_p.h>

using namespace qtmir;

TEST(ArgvHelperTest, StripsCorrectly)
{
    int argc = 5;
    const char *argv[6] = { "/usr/bin/unity8", "-fullscreen", "--debug",
                            "--platform-input-lib=/path/to/lib.so", "-testability" };

    const int filteredArgc = 2;
    const char *filteredArgv[3] = { "-fullscreen", "-testability" };

    editArgvToMatch(argc, const_cast<char**>(argv), filteredArgc, filteredArgv);

    EXPECT_EQ(argc, 3);
    EXPECT_EQ(argv[0], "/usr/bin/unity8");
    EXPECT_EQ(argv[1], "-fullscreen");
    EXPECT_EQ(argv[2], "-testability");
    EXPECT_EQ(argv[3], nullptr);
}

TEST(ArgvHelperTest, NothingToStrip)
{
    int argc = 4;
    const char *argv[5] = { "/usr/bin/unity8", "-fullscreen", "--multisample", "-testability" };

    const int filteredArgc = 3;
    const char *filteredArgv[4] = { "-fullscreen", "-testability", "--multisample" };

    editArgvToMatch(argc, const_cast<char**>(argv), filteredArgc, filteredArgv);

    EXPECT_EQ(argc, 4);
    EXPECT_EQ(argv[0], "/usr/bin/unity8");
    EXPECT_EQ(argv[1], "-fullscreen");
    EXPECT_EQ(argv[2], "-testability");
    EXPECT_EQ(argv[3], "--multisample");
    EXPECT_EQ(argv[4], nullptr);
}

TEST(ArgvHelperTest, NothingToDo)
{
    int argc = 1;
    const char *argv[2] = { "/usr/bin/unity8" };

    const int filteredArgc = 0;
    const char *filteredArgv[1] = { };

    editArgvToMatch(argc, const_cast<char**>(argv), filteredArgc, filteredArgv);

    EXPECT_EQ(argc, 1);
    EXPECT_EQ(argv[0], "/usr/bin/unity8");
    EXPECT_EQ(argv[1], nullptr);
}
