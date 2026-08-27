#pragma once

#include <compare>
#include <cstdint>

namespace UnrealVoxelSim::Ecs::Api
{

class RegistryScopeId final
{
  public:
    constexpr RegistryScopeId() noexcept = default;

    explicit constexpr RegistryScopeId(const std::uint32_t value) noexcept : m_Value(value)
    {
    }

    [[nodiscard]] constexpr bool IsValid() const noexcept
    {
        return m_Value != 0;
    }

    [[nodiscard]] constexpr std::uint32_t Value() const noexcept
    {
        return m_Value;
    }

    auto operator<=>(const RegistryScopeId &) const = default;

  private:
    std::uint32_t m_Value{};
};

}
