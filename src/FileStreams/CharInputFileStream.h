//
// <author> damirlj@yahoo.com
// Copyright (c) 2021. All rights reserved!
//

#ifndef INCLUDE_COMMON_FILE_CHARINPUTFILESTREAM_HXX_
#define INCLUDE_COMMON_FILE_CHARINPUTFILESTREAM_HXX_


#include "FileStream.h"

namespace utils::file
{
     // Strong (named) type - type alias
    using CharInputFileStream = InputFileStream<std::string>;
}

#endif /* INCLUDE_COMMON_FILE_CHARINPUTFILESTREAM_HXX_ */
