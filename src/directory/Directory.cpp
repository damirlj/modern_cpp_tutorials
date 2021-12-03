/*
 * Directory.cpp
 *
 *  Created on: Aug 22, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#include <algorithm>
#include <iterator>
#include <thread>


#include "Directory.h"


using namespace utils::files;
using namespace std;
using namespace std::filesystem;


Directory::Directory(path_t root) noexcept : m_root(root),
        m_pSyncThread(make_unique<utils::aot::AOThread>())
{
    m_pSyncThread->start();
}

Directory::~Directory()
{
    m_pSyncThread->stop();
}

void Directory::getAllEntries(std::filesystem::path root)
{
    for (const auto& entry : directory_iterator {root} )
    {
        //cout << "entry=" << entry << '\n';
        if (entry.is_directory())
        {
            m_directories.push_back(entry);
            getAllEntries(entry);
        }
        else if (entry.is_regular_file())
        {
            m_files.push_back(entry);
        }
    }
    //cout << "Exit: " << __func__ << '\n';
}


future<void> Directory::sync()
{
    return m_pSyncThread->emplace_enqueue(&Directory::getAllEntries, this, m_root);
}

future<void> Directory::forceSync()
{
    return sync();
}


Directory::entries_t Directory::getAllFiles()
{
    call_once(m_syncOnce, [this]{
       const auto f = sync();
       f.wait();
    });

    return m_files;
}


Directory::entries_t Directory::getSubdirectories()
{
   call_once(m_syncOnce, [this]{
       const auto f = sync();
       f.wait();
   });

   return m_directories;
}



namespace utils::files
{
    void printDirectoryEntries(Directory& directory)
    {
        auto subdirectories = directory.getSubdirectories();
        cout << "\n<Directories>:\n\n";
        for (const auto& dir : subdirectories)
        {
            cout << dir << '\n';
        }

        auto files = directory.getAllFiles();
        cout << "\n<Files>:\n\n";
        for (const auto& file : files)
        {
            cout << file << '\n';
        }
    }
}
