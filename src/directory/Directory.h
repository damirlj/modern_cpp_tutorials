/*
 * Directory.h
 *
 *  Created on: Aug 22, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef DS_FILES_DIRECTORY_H_
#define DS_FILES_DIRECTORY_H_

#include <filesystem>
#include <list>
#include <future>


#include "../AOT/AOThread_v2.h"

namespace utils::files
{
    class Directory final
    {
        public:

            using entry_t = std::filesystem::directory_entry;
            using entries_t = std::list<entry_t>;
            using path_t = std::filesystem::path;

            explicit Directory(path_t root) noexcept;
            ~Directory();

            std::future<void> forceSync();

            entries_t getSubdirectories();
            entries_t getAllFiles();

        private:

            /**
             * Cache the entries (directories/files) into memory
             */
            std::future<void> sync();
            void getAllEntries(std::filesystem::path root);

        private:

            std::filesystem::path m_root;

            entries_t m_directories;
            entries_t m_files;
            std::once_flag m_syncOnce;

            std::unique_ptr<utils::aot::AOThread> m_pSyncThread;

    };

    void printDirectoryEntries(const Directory& directory);
}

#endif /* DS_FILES_DIRECTORY_H_ */
