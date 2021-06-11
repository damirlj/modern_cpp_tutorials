//
// <author> damirlj@yahoo.com
// Copyright (c) 2021. All rights reserved!
//

#ifndef FILE_BINARYOUTPUTSTREAM_H_
#define FILE_BINARYOUTPUTSTREAM_H_


#include "FileStream.h"

namespace utils::file
{
    // Strong (named) type - type alias
    using BinaryOutputFileStream = OutputFileStream<uint8_t>; //std::byte
}

#endif /* FILE_BINARYOUTPUTSTREAM_H_ */
