#pragma once

#include "UnrealVoxelSim/Ecs/Api/RegistryScopeId.h"

#include <compare>
#include <cstdint>
#include <type_traits>

namespace UnrealVoxelSim::Ecs::Api
{

namespace Detail
{
class EntityIdAccess;
}

class EntityId final
{
  public:
    constexpr EntityId() noexcept = default;

    [[nodiscard]] constexpr bool IsValid() const noexcept
    {
        return Value_ != 0;
    }

    [[nodiscard]] constexpr RegistryScopeId Scope() const noexcept
    {
        return RegistryScopeId{static_cast<std::uint32_t>(Value_ >> 32U)};
    }

    auto operator<=>(const EntityId &) const = default;

  private:
    friend class Detail::EntityIdAccess;

    explicit constexpr EntityId(const std::uint64_t value) noexcept : Value_(value)
    {
    }

    std::uint64_t Value_{};
};

static_assert(sizeof(EntityId) == sizeof(std::uint64_t));
static_assert(std::is_trivially_copyable_v<EntityId>);

} // namespace UnrealVoxelSim::Ecs::Api
