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
			return m_Value != 0;
		}

		[[nodiscard]] constexpr RegistryScopeId Scope() const noexcept
		{
			return RegistryScopeId{static_cast<std::uint32_t>(m_Value >> 32U)};
		}

		auto operator<=>(const EntityId&) const = default;

	private:
		friend class Detail::EntityIdAccess;

		explicit constexpr EntityId(const std::uint64_t value) noexcept : m_Value(value)
		{
		}

		std::uint64_t m_Value{};
	};

	static_assert(sizeof(EntityId) == sizeof(std::uint64_t));
	static_assert(std::is_trivially_copyable_v<EntityId>);
}
