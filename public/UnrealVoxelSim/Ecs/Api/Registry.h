#pragma once

#include "UnrealVoxelSim/Ecs/Api/ComponentOperationError.h"
#include "UnrealVoxelSim/Ecs/Api/EntityId.h"
#include "UnrealVoxelSim/Ecs/Api/EntityOperationError.h"
#include "UnrealVoxelSim/Ecs/Api/RegistryScopeId.h"

#include <concepts>
#include <expected>
#include <functional>
#include <type_traits>
#include <utility>

namespace UnrealVoxelSim::Ecs::Api
{
	template <typename TRegistry>
	concept Registry = requires(TRegistry& registry,
	                            const TRegistry& constRegistry,
	                            EntityId entity,
	                            typename TRegistry::CommandBuffer& commandBuffer)
	{
		typename TRegistry::CommandBuffer;
		typename TRegistry::CommitResult;

		{ constRegistry.Scope() } -> std::same_as<RegistryScopeId>;
		{ registry.Create() } -> std::same_as<EntityId>;
		{ constRegistry.IsAlive(entity) } -> std::same_as<bool>;
		{ registry.Destroy(entity) } -> std::same_as<std::expected<void, EntityOperationError>>;
		{ registry.MakeCommandBuffer() } -> std::same_as<typename TRegistry::CommandBuffer>;
		{ registry.Commit(commandBuffer) } -> std::same_as<typename TRegistry::CommitResult>;
	};

	template <typename TRegistry, typename TComponent>
	concept ComponentRegistry = Registry<TRegistry> && requires(TRegistry& registry,
	                                                            const TRegistry& constRegistry,
	                                                            EntityId entity,
	                                                            TComponent component)
	{
		{ constRegistry.template Contains<TComponent>(entity) } -> std::same_as<bool>;
		{
			registry.template Assign<TComponent>(entity, std::move(component))
		} -> std::same_as<std::expected<void, ComponentOperationError>>;
		{
			registry.template Get<TComponent>(entity)
		} -> std::same_as<std::expected<std::reference_wrapper<TComponent>, ComponentOperationError>>;
		{
			constRegistry.template Get<TComponent>(entity)
		} -> std::same_as<std::expected<std::reference_wrapper<const TComponent>, ComponentOperationError>>;
		{ registry.template Remove<TComponent>(entity) } -> std::same_as<std::expected<void, ComponentOperationError>>;
	};

	template <typename TRegistry, typename TTag>
	concept TagRegistry = Registry<TRegistry> && std::is_empty_v<TTag> &&
		requires(TRegistry& registry, const TRegistry& constRegistry, EntityId entity)
		{
			{ constRegistry.template Contains<TTag>(entity) } -> std::same_as<bool>;
			{
				registry.template Assign<TTag>(entity)
			} -> std::same_as<std::expected<void, ComponentOperationError>>;
			{
				registry.template Remove<TTag>(entity)
			} -> std::same_as<std::expected<void, ComponentOperationError>>;
		};

	template <typename TRegistry, typename TQuery, typename TFunction>
	concept QueryableRegistry = Registry<TRegistry> && requires(TRegistry& registry, TQuery query, TFunction function)
	{
		registry.ForEach(query, function);
	};
}
