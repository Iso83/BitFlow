#pragma once

#include <BitFlow/core/helper/Exception.h>

namespace BitFlow::IO {

class IOException : public Core::CoreException {
  public:
    using CoreException::CoreException;
};

} // namespace BitFlow::IO

#define BF_IO_THROW(msg) throw ::BitFlow::IO::IOException(msg)