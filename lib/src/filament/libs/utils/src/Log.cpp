/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <utils/Log.h>

#include <string>
#include <utils/compiler.h>

#include "Debug.h"

namespace utils {

namespace io {

ostream& LogStream::flush() noexcept {
    Buffer& buf = getBuffer();
	Viry3D::Debug::LogString(buf.get(), false);
    buf.reset();
    return *this;
}

static LogStream cout(LogStream::Priority::DEBUG);
static LogStream cerr(LogStream::Priority::ERROR);
static LogStream cwarn(LogStream::Priority::WARNING);
static LogStream cinfo(LogStream::Priority::INFO);

} // namespace io


Loggers slog = {
        io::cout,   // debug
        io::cerr,   // error
        io::cwarn,  // warning
        io::cinfo   // info
};

} // namespace utils
