#include "orion-core/platform/win32/win32_filesystem.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <limits>

namespace orion
{
    PlatformFile::PlatformFile(HANDLE handle)
        : handle_(handle)
    {
    }

    namespace platform
    {
        namespace
        {
            constexpr auto max_bytes_at_time = std::numeric_limits<DWORD>::max();

            [[nodiscard]] DWORD to_win32_access(FileAccessFlags access_flags)
            {
                DWORD win32_flags = 0;
                if (!!(access_flags & FileAccessFlags::Read)) {
                    win32_flags |= GENERIC_READ;
                }
                if (!!(access_flags & FileAccessFlags::Write)) {
                    win32_flags |= GENERIC_WRITE;
                }
                return win32_flags;
            }

            [[nodiscard]] DWORD to_win32_share(FileShareFlags share_flags)
            {
                DWORD win32_flags = 0;
                if (!!(share_flags & FileShareFlags::Delete)) {
                    win32_flags |= FILE_SHARE_DELETE;
                }
                if (!!(share_flags & FileShareFlags::Read)) {
                    win32_flags |= FILE_SHARE_READ;
                }
                if (!!(share_flags & FileShareFlags::Write)) {
                    win32_flags |= FILE_SHARE_WRITE;
                }
                return win32_flags;
            }

            [[nodiscard]] DWORD to_win32_create_disposition(FileCreateMode create_mode)
            {
                switch (create_mode) {
                    case FileCreateMode::Create:
                        return CREATE_ALWAYS;
                    case FileCreateMode::CreateUnique:
                        return CREATE_NEW;
                    case FileCreateMode::Open:
                        return OPEN_ALWAYS;
                    case FileCreateMode::OpenExisting:
                        return OPEN_EXISTING;
                    case FileCreateMode::Truncate:
                        return TRUNCATE_EXISTING;
                }
                return 0;
            }

            [[nodiscard]] DWORD to_win32_attributes(FileAttributeFlags attribute_flags)
            {
                DWORD win32_attributes = 0;
                if (!!(attribute_flags & FileAttributeFlags::Normal)) {
                    win32_attributes |= FILE_ATTRIBUTE_NORMAL;
                }
                if (!!(attribute_flags & FileAttributeFlags::ReadOnly)) {
                    win32_attributes |= FILE_ATTRIBUTE_READONLY;
                }
                if (!!(attribute_flags & FileAttributeFlags::Hidden)) {
                    win32_attributes |= FILE_ATTRIBUTE_HIDDEN;
                }
                if (!!(attribute_flags & FileAttributeFlags::System)) {
                    win32_attributes |= FILE_ATTRIBUTE_SYSTEM;
                }
                if (!!(attribute_flags & FileAttributeFlags::Directory)) {
                    win32_attributes |= FILE_ATTRIBUTE_DIRECTORY;
                }
                if (!!(attribute_flags & FileAttributeFlags::Archive)) {
                    win32_attributes |= FILE_ATTRIBUTE_ARCHIVE;
                }
                if (!!(attribute_flags & FileAttributeFlags::Device)) {
                    win32_attributes |= FILE_ATTRIBUTE_DEVICE;
                }
                if (!!(attribute_flags & FileAttributeFlags::Temporary)) {
                    win32_attributes |= FILE_ATTRIBUTE_TEMPORARY;
                }
                if (!!(attribute_flags & FileAttributeFlags::Compressed)) {
                    win32_attributes |= FILE_ATTRIBUTE_COMPRESSED;
                }
                if (!!(attribute_flags & FileAttributeFlags::Virtual)) {
                    win32_attributes |= FILE_ATTRIBUTE_VIRTUAL;
                }
                return win32_attributes;
            }
        } // namespace

        PlatformFile* create_file(const fs::path& filepath, const FileOpenDesc& desc)
        {
            HANDLE handle = CreateFileW(
                filepath.c_str(),                              // Filename
                to_win32_access(desc.access_flags),            // Desired access
                to_win32_share(desc.share_flags),              // Sharing mode
                nullptr,                                       // Security attributes
                to_win32_create_disposition(desc.create_mode), // Create disposition
                to_win32_attributes(desc.attributes),          // Attributes
                nullptr);
            if (handle == INVALID_HANDLE_VALUE) {
                SPDLOG_LOGGER_ERROR(win32::logger(), "CreateFile() failed");
                throw Win32Error();
            }

            return new PlatformFile(handle);
        }

        void destroy_file(PlatformFile* platform_file)
        {
            if (platform_file != nullptr) {
                CloseHandle(platform_file->handle());
                delete platform_file;
            }
        }

        FileType get_file_type(const PlatformFile* platform_file)
        {
            switch (GetFileType(platform_file->handle())) {
                case FILE_TYPE_CHAR:
                    return FileType::Char;
                case FILE_TYPE_DISK:
                    return FileType::Disk;
                case FILE_TYPE_PIPE:
                    return FileType::Pipe;
                default:
                    return FileType::Unknown;
            }
        }

        std::size_t get_file_size(const PlatformFile* platform_file)
        {
            LARGE_INTEGER size;
            if (!GetFileSizeEx(platform_file->handle(), &size)) {
                throw Win32Error();
            }
            return size.QuadPart;
        }

        std::size_t read_file(PlatformFile* platform_file, std::span<char> out_bytes)
        {
            HANDLE handle = platform_file->handle();
            std::size_t total_bytes_read = 0;
            auto total_bytes_left = out_bytes.size();
            auto* pointer = out_bytes.data();
            while (total_bytes_left > 0) {
                const DWORD bytes_left = total_bytes_left > max_bytes_at_time ? max_bytes_at_time : total_bytes_left;
                DWORD bytes_read = 0;
                if (!ReadFile(handle, pointer, bytes_left, &bytes_read, nullptr)) {
                    throw Win32Error();
                }
                total_bytes_read += bytes_read;
                total_bytes_left -= bytes_read;
                pointer += bytes_read;
            }
            return total_bytes_read;
        }

        std::size_t write_file(PlatformFile* platform_file, std::span<const char> in_bytes)
        {
            HANDLE handle = platform_file->handle();
            std::size_t total_bytes_written = 0;
            auto total_bytes_left = in_bytes.size();
            auto* pointer = in_bytes.data();
            while (total_bytes_left > 0) {
                const DWORD bytes_left = total_bytes_left > max_bytes_at_time ? max_bytes_at_time : total_bytes_left;
                DWORD bytes_written = 0;
                if (!WriteFile(handle, in_bytes.data(), bytes_left, &bytes_written, nullptr)) {
                    throw Win32Error();
                }
                total_bytes_written += bytes_written;
                total_bytes_left -= bytes_written;
                pointer += bytes_written;
            }
            return total_bytes_written;
        }
    } // namespace platform
} // namespace orion
