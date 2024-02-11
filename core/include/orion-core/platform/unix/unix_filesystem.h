#pragma once

#include "orion-core/filesystem.h"

#include "orion-core/platform/unix/unix_platform.h"

namespace orion
{
    class PlatformFile
    {
    public:
        explicit PlatformFile(int file_descriptor);

        [[nodiscard]] int fd() const noexcept { return file_descriptor_; }

    private:
        int file_descriptor_;
    };

    namespace unix
    {
        [[nodiscard]] SyscallResult<int> open_file(const fs::path& filepath, const FileOpenDesc& desc);
        SyscallResult<void> close_file(int file_descriptor);
        SyscallResult<ssize_t> read_file(int fd, void* buf, size_t nbytes);
        SyscallResult<ssize_t> write_file(int fd, const void* buf, size_t nbytes);
    } // namespace unix
} // namespace orion
