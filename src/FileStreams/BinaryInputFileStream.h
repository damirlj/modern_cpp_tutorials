//
// <author> damirlj@yahoo.com
// Copyright (c) 2021. All rights reserved!
//

#ifndef INCLUDE_COMMON_FILE_BINARYINPUTFILESTREAM_HXX_
#define INCLUDE_COMMON_FILE_BINARYINPUTFILESTREAM_HXX_

#include "FileStream.h"

namespace utils::file
{
    // Strong (named) type - type alias
    using BinaryInputFileStream = InputFileStream<uint8_t>; // std::byte
}


#endif /* INCLUDE_COMMON_FILE_BINARYINPUTFILESTREAM_HXX_ */
