#pragma once

#include <BitFlow/engine/core/helper/Exception.h>

namespace BitFlow::Engine::IO {

class IOException : public Core::CoreException {
public:
    using CoreException::CoreException;
};

} // namespace BitFlow::Engine::IO

#define BF_IO_THROW(msg) throw ::BitFlow::Engine::IO::IOException(msg)
