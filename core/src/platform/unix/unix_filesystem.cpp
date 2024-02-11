#include "orion-core/platform/unix/unix_filesystem.h"

#include "orion-core/platform/unix/unix_platform.h"

#include "orion-utils/assertion.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace orion
{
    PlatformFile::PlatformFile(int file_descriptor)
        : file_descriptor_(file_descriptor)
    {
    }

    namespace unix
    {
        namespace
        {
            int get_access_flags(FileAccessFlags access_flags)
            {
                switch (access_flags) {
                    case FileAccessFlags::ReadWrite:
                        return O_RDWR;
                    case FileAccessFlags::Read:
                        return O_RDONLY;
                    case FileAccessFlags::Write:
                        return O_WRONLY;
                }
                ORION_ASSERT(!"invalid access flag");
                return -1;
            }

            int get_open_mode(FileCreateMode create_mode)
            {
                switch (create_mode) {
                    case FileCreateMode::Create:
                        return O_CREAT | O_TRUNC;
                    case FileCreateMode::CreateUnique:
                        return O_CREAT | O_EXCL;
                    case FileCreateMode::Open:
                        return O_CREAT;
                    case FileCreateMode::OpenExisting:
                        return 0;
                    case FileCreateMode::Truncate:
                        return O_TRUNC;
                }
            }

            int get_open_flags(const FileOpenDesc& desc)
            {
                const int access_flags = get_access_flags(desc.access_flags);
                const int open_mode = get_open_mode(desc.create_mode);
                return access_flags | open_mode;
            }
        } // namespace

        SyscallResult<int> open_file(const fs::path& filepath, const FileOpenDesc& desc)
        {
            if (int fd = open(filepath.c_str(), get_open_flags(desc)); fd != -1) {
                return fd;
            }
            return errno_unexpect();
        }

        SyscallResult<void> close_file(int file_descriptor)
        {
            if (close(file_descriptor) == -1) {
                return errno_unexpect();
            }
            return {};
        }

        SyscallResult<ssize_t> read_file(int fd, void* buf, size_t nbytes)
        {
            const auto nread = read(fd, buf, nbytes);
            if (nread == -1) {
                return errno_unexpect();
            }
            return nread;
        }

        SyscallResult<ssize_t> write_file(int fd, const void* buf, size_t nbytes)
        {
            const auto nwrote = write(fd, buf, nbytes);
            if (nwrote == -1) {
                return errno_unexpect();
            }
            return nwrote;
        }
    } // namespace unix

    namespace platform
    {
        namespace
        {
            [[nodiscard]] FileType to_orion_file_type(mode_t mode)
            {
                if (mode & S_IFCHR) {
                    return FileType::Char;
                }
                if (mode & S_IFIFO) {
                    return FileType::Pipe;
                }
                return FileType::Unknown;
            }
        } // namespace

        PlatformFile* create_file(const fs::path& filepath, const FileOpenDesc& desc)
        {
            auto result = unix::open_file(filepath, desc);
            if (result.has_value()) {
                return new PlatformFile{result.value()};
            }
            return nullptr;
        }

        void destroy_file(PlatformFile* platform_file)
        {
            if (platform_file != nullptr) {
                ORION_ASSERT(unix::close_file(platform_file->fd()));
            }
        }

        FileType get_file_type(const PlatformFile* platform_file)
        {
            struct stat file_stat;
            if (fstat(platform_file->fd(), &file_stat) == -1) {
                SPDLOG_LOGGER_ERROR(unix::logger(), "{}", unix::errno_string());
                return FileType::Unknown;
            }
            return to_orion_file_type(file_stat.st_mode);
        }

        std::size_t get_file_size(const PlatformFile* platform_file)
        {
            struct stat file_stat;
            if (fstat(platform_file->fd(), &file_stat) == -1) {
                SPDLOG_LOGGER_ERROR(unix::logger(), "{}", unix::errno_string());
                return 0;
            }
            return static_cast<size_t>(file_stat.st_size);
        }

        std::size_t read_file(PlatformFile* platform_file, std::span<char> out_bytes)
        {
            if (platform_file == nullptr) {
                return 0;
            }

            const auto result = unix::read_file(platform_file->fd(), out_bytes.data(), out_bytes.size_bytes());
            if (!result) {
                SPDLOG_LOGGER_ERROR(unix::logger(), "{}", unix::errno_string());
                return 0;
            }
            return static_cast<std::size_t>(result.value());
        }

        std::size_t write_file(PlatformFile* platform_file, std::span<const char> in_bytes)
        {
            if (platform_file == nullptr) {
                return 0;
            }

            const auto result = unix::write_file(platform_file->fd(), in_bytes.data(), in_bytes.size_bytes());
            if (!result) {
                SPDLOG_LOGGER_ERROR(unix::logger(), "{}", unix::errno_string());
                return 0;
            }
            return static_cast<std::size_t>(result.value());
        }
    } // namespace platform
} // namespace orion
