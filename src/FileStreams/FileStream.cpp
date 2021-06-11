//
// <author> damirlj@yahoo.com
// Copyright (c) 2021. All rights reserved!
//

#include "FileStream.h"

using namespace utils::file;

FileStream::FileStream(const std::string& path, std::ios_base::openmode mode) noexcept
{
    m_file.open(path, mode);
}

FileStream::~FileStream()
{
    m_file.close();
}


bool FileStream::isOpen() const
{
    return m_file.is_open();
}

std::size_t FileStream::size() const
{
    const auto pos =  m_file.tellg(); //current file position

    m_file.seekg(0, std::ios::end);
    const auto size = static_cast<std::size_t>(m_file.tellg());

    m_file.seekg(pos);

    return size;
}
