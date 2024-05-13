#pragma once

namespace orion
{
    // Forward declare
    class Effect;

    class Material
    {
    public:
        explicit Material(Effect* effect);

        [[nodiscard]] Effect* effect() const noexcept { return effect_; }

    private:
        Effect* effect_;
    };
} // namespace orion
