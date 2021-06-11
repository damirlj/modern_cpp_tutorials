//
// <author> damirlj@yahoo.com
// Copyright (c) 2021. All rights reserved!
//

#ifndef INCLUDE_COMMON_FILE_CHAROUTPUTFILESTREAM_H_
#define INCLUDE_COMMON_FILE_CHAROUTPUTFILESTREAM_H_

#include "FileStream.h"

namespace utils::file
{
    // Strong (named) type - type alias
    using CharOutputFileStream = OutputFileStream<std::string>;
}


#endif /* INCLUDE_COMMON_FILE_CHAROUTPUTFILESTREAM_H_ */
