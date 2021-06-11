//
// <author> damirlj@yahoo.com
// Copyright (c) 2021. All rights reserved!
//

#ifndef FILE_STREAMS_H_
#define FILE_STREAMS_H_


#include <errno.h>
#include <string.h>

#include <fstream>
#include <string>
#include <vector>


namespace utils::file
{
        /**
         * RAII wrapper around the std::fstream
         */
        class FileStream
        {
            public:


                /**
                 * C-tor
                 * Create the file stream.
                 * Open the file.
                 *
                 * @param path  The absolute file path
                 * @param mode  Open mode, which specify the type of the file stream and the
                 * way how it should be created
                 */
                FileStream(const std::string& path
                        , std::ios_base::openmode mode) noexcept;

                // Copy operation forbidden

                FileStream(const FileStream& ) = delete;
                FileStream& operator = (const FileStream& ) = delete;

                bool isOpen() const;

                std::size_t size() const;

                /**
                 * D-tor
                 *
                 * Close the file stream
                 */
                virtual ~FileStream();

            protected:

                mutable std::fstream m_file;
        };

        template <class T>
        class OutputFileStream : public FileStream
        {
            public:

                explicit OutputFileStream(const std::string& path) noexcept :
                    FileStream(path, std::ofstream::out)
                {}

                OutputFileStream(const std::string& path, std::ios_base::openmode mode) noexcept :
                    FileStream(path, std::ofstream::out | mode)
                {}

                virtual ~OutputFileStream() override = default;

                using data_t = T;
                using chunk_t = std::vector<data_t>;

                virtual void write(const chunk_t& data)
                {
                    writeData(data);
                }

                virtual void write(chunk_t&& data)
                {
                    writeData(std::move(data));
                }

            private:

                    template<class T1, typename = std::enable_if_t<std::is_constructible<chunk_t, T1>::value>>
                    void writeData(T1&& data)
                    {
                        auto&& rdata = std::forward<T1>(data);//universal reference
                        m_file.write(reinterpret_cast<const char*>(&rdata[0]), rdata.size());
                    }

        };//class OutputFileStream<T>



        template <class T>
        class InputFileStream: public FileStream
        {
            public:

                explicit InputFileStream(const std::string& path) noexcept:
                    FileStream(path, std::ifstream::in)
                {}

                InputFileStream(const std::string& path, std::ios_base::openmode mode) noexcept :
                    FileStream(path, std::ifstream::in | mode)
                {}

                virtual ~InputFileStream() override = default;

                using data_t = T;
                using chunk_t = std::vector<data_t>;

                /**
                 * Read entire file into memory
                 * @note It will throw std::runtime_error in case that operation on
                 * the input file stream (std::ifstream) is somehow failed
                 */
                virtual chunk_t readAll();

        };//class InputFileStream<T>

        template <class T>
        inline typename InputFileStream<T>::chunk_t
        InputFileStream<T>::readAll()
        {
             const auto file_size = size();

             chunk_t buffer(file_size);

             m_file.seekg(0);//rewind the position to the beginning of the stream
             if (!m_file.read(reinterpret_cast<char*>(&buffer[0]), file_size) )
                 throw std::runtime_error(strerror(errno));

             return buffer;
        }
}//namespace:utils::file

#endif /*FILE_STREAMS_H_*/
