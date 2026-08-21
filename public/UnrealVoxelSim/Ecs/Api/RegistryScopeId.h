#pragma once

#include <compare>
#include <cstdint>

namespace UnrealVoxelSim::Ecs::Api
{

class RegistryScopeId final
{
  public:
    constexpr RegistryScopeId() noexcept = default;

    explicit constexpr RegistryScopeId(const std::uint32_t value) noexcept : Value_(value)
    {
    }

    [[nodiscard]] constexpr bool IsValid() const noexcept
    {
        return Value_ != 0;
    }

    [[nodiscard]] constexpr std::uint32_t Value() const noexcept
    {
        return Value_;
    }

    auto operator<=>(const RegistryScopeId &) const = default;

  private:
    std::uint32_t Value_{};
};

} // namespace UnrealVoxelSim::Ecs::Api
