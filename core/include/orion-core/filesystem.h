#pragma once

#include "orion-utils/bitflag.h"

#include <filesystem>
#include <memory>
#include <span>
#include <vector>

namespace orion
{
    namespace fs = std::filesystem;

    ORION_BITFLAG(FileAccessFlags, std::uint8_t){
        Read = 0x1,
        Write = 0x2,
        ReadWrite = Read | Write,
    };

    ORION_BITFLAG(FileShareFlags, std::uint8_t){
        None = 0,
        Delete = 0x1,
        Read = 0x2,
        Write = 0x4,
    };

    enum class FileCreateMode {
        Create,
        CreateUnique,
        Open,
        OpenExisting,
        Truncate,
    };

    ORION_BITFLAG(FileAttributeFlags, std::uint32_t){
        Normal = 0x1,
        ReadOnly = 0x2,
        Hidden = 0x4,
        System = 0x8,
        Directory = 0x10,
        Archive = 0x20,
        Device = 0x40,
        Temporary = 0x90,
        Compressed = 0x100,
        Virtual = 0x200,
    };

    struct FileOpenDesc {
        FileAccessFlags access_flags;
        FileShareFlags share_flags;
        FileCreateMode create_mode;
        FileAttributeFlags attributes = FileAttributeFlags::Normal;
    };

    enum class FileType {
        Unknown,
        Char,
        Disk,
        Pipe,
    };

    class PlatformFile;
    namespace platform
    {
        [[nodiscard]] PlatformFile* create_file(const fs::path& filepath, const FileOpenDesc& desc);
        void destroy_file(PlatformFile* platform_file);
        [[nodiscard]] FileType get_file_type(const PlatformFile* platform_file);
        [[nodiscard]] std::size_t get_file_size(const PlatformFile* platform_file);
        std::size_t read_file(PlatformFile* platform_file, std::span<char> out_bytes);
        std::size_t write_file(PlatformFile* platform_file, std::span<const char> in_bytes);
    } // namespace platform

    using PlatformFilePtr = std::unique_ptr<PlatformFile, decltype(&platform::destroy_file)>;

    class File
    {
    public:
        File(const fs::path& filepath, const FileOpenDesc& desc);

        std::size_t read(std::span<char> out_bytes);
        std::size_t write(std::span<const char> in_bytes);

        std::vector<char> read_all();

        [[nodiscard]] FileType type() const noexcept { return type_; }
        [[nodiscard]] std::size_t size() const;

        [[nodiscard]] bool can_read() const noexcept;
        [[nodiscard]] bool can_write() const noexcept;

    private:
        PlatformFilePtr platform_file_;
        FileOpenDesc open_desc_;
        FileType type_;
    };

    [[nodiscard]] File create_input_file(const fs::path& filepath);
    [[nodiscard]] File create_output_file(const fs::path& filepath);
} // namespace orion
