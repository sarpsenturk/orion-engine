#include "orion-core/filesystem.h"

#include "orion-utils/assertion.h"

namespace orion
{
    File::File(const fs::path& filepath, const FileOpenDesc& desc)
        : platform_file_(platform::create_file(filepath, desc), &platform::destroy_file)
        , open_desc_(desc)
        , type_(platform::get_file_type(platform_file_.get()))
    {
    }

    std::size_t File::read(std::span<char> out_bytes)
    {
        ORION_EXPECTS(platform_file_ != nullptr);
        ORION_ASSERT(can_read());
        return platform::read_file(platform_file_.get(), out_bytes);
    }

    std::size_t File::write(std::span<const char> in_bytes)
    {
        ORION_EXPECTS(platform_file_ != nullptr);
        ORION_ASSERT(can_write());
        return platform::write_file(platform_file_.get(), in_bytes);
    }

    std::vector<char> File::read_all()
    {
        std::vector<char> bytes(size());
        read(bytes);
        return bytes;
    }

    std::size_t File::size() const
    {
        ORION_EXPECTS(platform_file_ != nullptr);
        return platform::get_file_size(platform_file_.get());
    }

    bool File::can_read() const noexcept
    {
        return !!(open_desc_.access_flags & FileAccessFlags::Read);
    }

    bool File::can_write() const noexcept
    {
        return !!(open_desc_.access_flags & FileAccessFlags::Write);
    }

    File create_input_file(const fs::path& filepath)
    {
        return File(filepath, {.access_flags = FileAccessFlags::Read, .share_flags = FileShareFlags::Read, .create_mode = FileCreateMode::OpenExisting});
    }

    File create_output_file(const fs::path& filepath)
    {
        return File(filepath, {.access_flags = FileAccessFlags::Write, .share_flags = FileShareFlags::Read, .create_mode = FileCreateMode::Create});
    }
} // namespace orion
