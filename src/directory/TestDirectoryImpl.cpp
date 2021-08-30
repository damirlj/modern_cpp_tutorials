/*
 * TestDirectoryImpl.cpp
 *
 *  Created on: Aug 22, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#include <bits/stdc++.h>


#include "Directory.h"
#include "TestDirectoryImpl.h"

namespace utils::files
{
    int testDirectoryImpl()
    {
        using namespace std;
        using namespace std::filesystem;

        const vector<path> paths =
        {
            "./src/directory",
        };

        for (const auto& p : paths)
        {
            cout << "Root: " << p << '\n';
            Directory directory {p};
            printDirectoryEntries(directory);
        }

        return 0;

    }
}

