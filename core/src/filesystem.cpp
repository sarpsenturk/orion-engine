#include "orion-core/filesystem.h"

#include <filesystem>
#include <system_error>

namespace orion
{
    namespace
    {
        std::FILE* file_open(const char* path, const char* mode)
        {
            std::FILE* file = std::fopen(path, mode);
            if (file == nullptr) {
                // TODO: Log file open failed
                throw std::system_error{errno, std::generic_category()};
            }
            return file;
        }

        void file_close(std::FILE* file)
        {
            std::fclose(file);
        }

        void file_noop(std::FILE* /*unused*/)
        {
        }
    } // namespace

    FilePath::FilePath(const char* path)
        : path_(path)
    {
    }

    FilePath::FilePath(std::string path)
        : path_(std::move(path))
    {
    }

    FilePath& FilePath::append(const FilePath& other)
    {
        path_ += (separator + other.path_);
        return *this;
    }

    FilePath operator/(const FilePath& lhs, const FilePath& rhs)
    {
        auto result = lhs;
        result.append(rhs);
        return result;
    }

    std::string format_as(const FilePath& path)
    {
        return path.string();
    }

    File::File(std::FILE* file)
        : file_(file, &file_noop)
    {
    }

    File::File(FilePath path, const char* mode)
        : file_(file_open(path.c_str(), mode), &file_close)
        , path_(std::move(path))
    {
    }

    std::size_t File::size() const
    {
        return std::filesystem::file_size(path_.c_str());
    }

    std::size_t File::read(std::span<std::byte> outbytes) const
    {
        return std::fread(outbytes.data(), sizeof(std::byte), outbytes.size(), file_.get());
    }

    std::size_t File::write(std::span<const std::byte> inbytes)
    {
        return std::fwrite(inbytes.data(), sizeof(std::byte), inbytes.size(), file_.get());
    }

    void File::flush()
    {
        if (!std::fflush(file_.get())) {
            throw std::system_error{errno, std::generic_category()};
        }
    }

    std::vector<std::byte> File::read_all() const
    {
        std::vector<std::byte> bytes(size());
        read(bytes);
        return bytes;
    }

    std::string File::read_all_str() const
    {
        std::string string(size(), ' ');
        read(std::as_writable_bytes(std::span{string}));
        return string;
    }

    File input_file(FilePath path)
    {
        return File{std::move(path), "r"};
    }

    File output_file(FilePath path)
    {
        return File{std::move(path), "w"};
    }

    File binary_input_file(FilePath path)
    {
        return File{std::move(path), "rb"};
    }

    File binary_output_file(FilePath path)
    {
        return File{std::move(path), "wb"};
    }

    File* stdinput()
    {
        static auto file = File(stdin);
        return &file;
    }

    File* stdoutput()
    {
        static auto file = File(stdout);
        return &file;
    }

    File* stderror()
    {
        static auto file = File(stderr);
        return &file;
    }

    namespace fs
    {
        bool exists(const FilePath& path)
        {
            return std::filesystem::exists(path.string());
        }
    } // namespace fs
} // namespace orion
