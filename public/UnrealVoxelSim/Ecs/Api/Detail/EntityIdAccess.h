#pragma once

#include "UnrealVoxelSim/Ecs/Api/EntityId.h"

#include <cstdint>

namespace UnrealVoxelSim::Ecs::Api::Detail
{

class EntityIdAccess final
{
  public:
    [[nodiscard]] static constexpr EntityId Create(const RegistryScopeId scope, const std::uint32_t localValue) noexcept
    {
        return EntityId{(static_cast<std::uint64_t>(scope.Value()) << 32U) | localValue};
    }

    [[nodiscard]] static constexpr std::uint32_t LocalValue(const EntityId entity) noexcept
    {
        return static_cast<std::uint32_t>(Value(entity));
    }

    [[nodiscard]] static constexpr std::uint64_t Value(const EntityId entity) noexcept
    {
        return entity.m_Value;
    }
};

}
