#pragma once

#include <cstddef>
#include <cstdio>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace orion
{
    using FileDeleter = void (*)(std::FILE*);
    using FilePtr = std::unique_ptr<std::FILE, FileDeleter>;

    class FilePath
    {
    public:
        static constexpr auto separator = '/';

        FilePath() = default;
        explicit(false) FilePath(const char* path);
        explicit(false) FilePath(std::string path);

        [[nodiscard]] const std::string& string() const { return path_; }
        [[nodiscard]] const char* c_str() const { return path_.c_str(); }

        FilePath& append(const FilePath& other);

        friend FilePath operator/(const FilePath& lhs, const FilePath& rhs);

    private:
        std::string path_;
    };

    [[nodiscard]] std::string format_as(const FilePath& path);

    class File
    {
    public:
        explicit File(std::FILE* file);
        File(FilePath path, const char* mode);

        [[nodiscard]] std::size_t size() const;

        std::size_t read(std::span<std::byte> outbytes) const;
        std::size_t write(std::span<const std::byte> inbytes);

        void flush();

        std::vector<std::byte> read_all() const;
        std::string read_all_str() const;

    private:
        FilePtr file_;
        FilePath path_;
    };

    [[nodiscard]] File input_file(FilePath path);
    [[nodiscard]] File output_file(FilePath path);
    [[nodiscard]] File binary_input_file(FilePath path);
    [[nodiscard]] File binary_output_file(FilePath path);

    [[nodiscard]] File* stdinput();
    [[nodiscard]] File* stdoutput();
    [[nodiscard]] File* stderror();

    namespace fs
    {
        [[nodiscard]] bool exists(const FilePath& path);
    }
} // namespace orion
