/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef ARGVHELPER_P_H
#define ARGVHELPER_P_H

#include <cassert>

namespace qtmir {

// Function to edit argv to strip out unmatched entries of targetArray, so both arrays have identical contents
// Note: Mir parses out arguments that it understands, but it also removes argv[0] (bug pad.lv/1511509)
// so need to ensure argv[0] is the binary name as usual.
void editArgvToMatch(int &argcToEdit, char** argvToEdit, int targetCount, const char* const targetArray[])
{
    // Make copy of the argv array of pointers, as we will be editing the original
    const size_t arraySize = (argcToEdit + 1) * sizeof(char*);
    char** argvCopy = static_cast<char**>(malloc(arraySize));
    memcpy(argvCopy, argvToEdit, arraySize);

    int k=1; // index of argv we want to edit - note we'll leave argv[0] alone
    for (int i=0; i<targetCount; i++) { // taking each argument Mir did not parse out
        for (int j=1; j<argcToEdit; j++) { // find pointer to same argument in argvCopy (leave arg[0] out)
            if (strcmp(targetArray[i], argvCopy[j]) == 0) {
                argvToEdit[k] = const_cast<char*>(argvCopy[j]); // edit argv to position that argument to match argv2.
                k++;
                break;
            }
        }
    }
    assert(k == targetCount);
    argvToEdit[k] = nullptr;
    free(argvCopy);
    argcToEdit = targetCount+1; // now argv and targetArray should have list the same strings.
}

} // namespace qtmir

#endif // ARGVHELPER_P_H
