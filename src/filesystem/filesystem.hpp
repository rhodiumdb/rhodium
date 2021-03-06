// Copyright 2021 The RDSS Authors
// Copyright 2020 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RDSS_FILESYSTEM_H_
#define RDSS_FILESYSTEM_H_

#include <errno.h>
#include <fcntl.h>
#include <string>
#include <filesystem>

#include <absl/status/status.h>
#include <absl/status/statusor.h>

namespace rdss {

namespace {

absl::Status ErrNoToStatusWithFilename(
    int errno_value,
    const std::filesystem::path& file_name) {
    std::stringstream ss;

    ss << "Error Code: " << errno_value;
    ss << "\nPath was: " << file_name << "\n";
    return absl::InternalError(ss.str());
}

}  // namespace

// Reads and returns the contents of the file `file_name`.
//
// Typical return codes (not guaranteed exhaustive):
//  * StatusCode::kOk
//  * StatusCode::kPermissionDenied (file not readable)
//  * StatusCode::kNotFound (no such file)
//  * StatusCode::kUnknown (a Read or Open error occurred)
absl::StatusOr<std::string> GetFileContents(
    const std::filesystem::path& file_name) {
    // Use POSIX C APIs instead of C++ iostreams to avoid exceptions.
    std::string result;

    int fd = open(file_name.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        return ErrNoToStatusWithFilename(errno, file_name);
    }

    char buf[4096];
    while (ssize_t n = read(fd, buf, sizeof(buf))) {
        if (n < 0) {
            if (errno == EAGAIN) {
                continue;
            }
            close(fd);
            return ErrNoToStatusWithFilename(errno, file_name);
        }
        result.append(buf, n);
    }

    if (close(fd) != 0) {
        return ErrNoToStatusWithFilename(errno, file_name);
    }
    return std::move(result);
}

// Writes the data provided in `content` to the file `file_name`, overwriting
// any existing content. Fails if directory does not exist.
//
// NOTE: Will return OK iff all of the data in `content` was written.
// May write some of the data and return an error.
//
// WARNING: The file update is NOT guaranteed to be atomic.
//
// Typical return codes (not guaranteed exhaustive):
//  * StatusCode::kOk
//  * StatusCode::kPermissionDenied (file not writable)
//  * StatusCode::kUnknown (a Write or Open error occurred)
absl::Status SetFileContents(const std::filesystem::path& file_name,
                             absl::string_view content) {
    // Use POSIX C APIs instead of C++ iostreams to avoid exceptions.
    int fd = open(file_name.c_str(), O_WRONLY | O_CREAT | O_CLOEXEC, 0664);
    if (fd == -1) {
        return ErrNoToStatusWithFilename(errno, file_name);
    }

    // Clear existing contents
    if (ftruncate(fd, 0) == -1) {
        return ErrNoToStatusWithFilename(errno, file_name);
    }

    ssize_t written = 0;
    while (written < content.size()) {
        ssize_t n = write(fd, content.data() + written,
                          content.size() - written);
        if (n < 0) {
            if (errno == EAGAIN) {
                continue;
            }
            close(fd);
            return ErrNoToStatusWithFilename(errno, file_name);
        }
        written += n;
    }

    if (close(fd) != 0) {
        return ErrNoToStatusWithFilename(errno, file_name);
    }
    return absl::OkStatus();
}

} // namespace rdss

#endif  // RDSS_FILESYSTEM_H_
