#pragma once

#include "UnrealVoxelSim/Ecs/Api/EntityId.h"

#include <cstdint>

namespace UnrealVoxelSim::Ecs::Api::Backend
{
	/**
	 * @brief Privileged construction and decomposition of opaque entity identifiers for ECS backends.
	 *
	 * @details Domain and application code intentionally cannot construct an EntityId from an integer or extract its
	 * backend-local representation. EntityIdAccess is the narrow backend integration surface allowed to cross that
	 * boundary. A registry implementation uses Create() when translating a backend entity handle into the portable
	 * EntityId contract, and GetValue() when translating that contract back to its own handle type.
	 *
	 * EntityId currently stores the RegistryScopeId in the upper 32 bits and the backend-local value in the lower 32
	 * bits. The scope prevents an identifier produced by one registry from being accepted by another registry. The
	 * lower value remains opaque to Ecs.Api; a backend may use it for an index, generation bits, or another compact
	 * handle representation.
	 *
	 * This type is public only because independently compiled ECS backend modules require it. It is not a
	 * general-purpose entity factory and must not be used by domain systems, presentation adapters, or persistence
	 * encoders. In particular, the packed runtime representation is session-local and is never a durable or network
	 * identity.
	 */
	class EntityIdAccess final
	{
	public:
		/**
		 * @brief Creates a portable entity identifier from one registry scope and one backend-local value.
		 *
		 * @param scope Scope of the registry that owns the backend entity.
		 * @param localValue Backend-specific 32-bit entity handle representation.
		 * @return An EntityId containing the scope in its upper bits and localValue in its lower bits.
		 *
		 * @note This function performs representation translation only. The owning registry remains responsible for
		 * validating the scope and determining whether the resulting entity is alive.
		 */
		[[nodiscard]] static constexpr EntityId Create(const RegistryScopeId scope,
													   const std::uint32_t localValue) noexcept
		{
			return EntityId{(static_cast<std::uint64_t>(scope.Value()) << 32U) | localValue};
		}

		/**
		 * @brief Extracts the backend-local value from a portable entity identifier.
		 *
		 * @param entity Identifier previously associated with an ECS backend.
		 * @return The lower 32-bit backend representation, without the RegistryScopeId.
		 *
		 * @warning Extracting the value does not prove that entity belongs to a particular registry or is still alive.
		 * A backend must validate entity.Scope() and its own entity-liveness rules before operating on the returned
		 * value.
		 */
		[[nodiscard]] static constexpr std::uint32_t GetValue(const EntityId entity) noexcept
		{
			return static_cast<std::uint32_t>(entity.m_Value);
		}
	};
}
