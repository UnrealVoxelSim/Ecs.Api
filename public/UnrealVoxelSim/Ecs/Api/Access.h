#pragma once

#include "UnrealVoxelSim/Ecs/Api/Query.h"
#include "UnrealVoxelSim/Ecs/Api/Registry.h"

#include <concepts>
#include <expected>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace UnrealVoxelSim::Ecs::Api
{
	template <typename... TComponents>
	struct Structural final
	{
	};

	enum class EntityAuthority
	{
		Observe,
		Own,
	};

	template <typename TRead = Read<>,
	          typename TWrite = Write<>,
	          typename TStructural = Structural<>,
	          EntityAuthority TEntities = EntityAuthority::Observe>
	struct Permissions final
	{
		using ReadAccess = TRead;
		using WriteAccess = TWrite;
		using StructuralAccess = TStructural;

		static constexpr EntityAuthority Entities = TEntities;
	};

	namespace Detail
	{
		template <typename T, typename TList>
		inline constexpr bool ContainsType = false;

		template <typename T, template <typename...> typename TList, typename... TItems>
		inline constexpr bool ContainsType<T, TList<TItems...>> = (std::same_as<T, TItems> || ...);

		template <typename TComponent>
		struct ComponentOperations final
		{
			using ConstResult = std::expected<std::reference_wrapper<const TComponent>, ComponentOperationError>;
			using MutableResult = std::expected<std::reference_wrapper<TComponent>, ComponentOperationError>;
			using MutationResult = std::expected<void, ComponentOperationError>;

			bool (*Contains)(const void*, EntityId) noexcept;
			ConstResult (*GetConst)(const void*, EntityId);
			MutableResult (*GetMutable)(void*, EntityId);
			MutationResult (*Assign)(void*, EntityId, TComponent);
			MutationResult (*Remove)(void*, EntityId);
		};

		template <typename TComponent, Registry TRegistry>
		[[nodiscard]] constexpr ComponentOperations<TComponent> BindComponentOperations() noexcept
		{
			return {
				[](const void* registry, const EntityId entity) noexcept
				{
					return static_cast<const TRegistry*>(registry)->template Contains<TComponent>(entity);
				},
				[](const void* registry, const EntityId entity)
				{
					if constexpr (std::is_empty_v<TComponent>)
						return typename ComponentOperations<TComponent>::ConstResult{
							std::unexpected{ComponentOperationError::NotPresent}
						};
					else
						return static_cast<const TRegistry*>(registry)->template Get<TComponent>(entity);
				},
				[](void* registry, const EntityId entity)
				{
					if constexpr (std::is_empty_v<TComponent>)
						return typename ComponentOperations<TComponent>::MutableResult{
							std::unexpected{ComponentOperationError::NotPresent}
						};
					else
						return static_cast<TRegistry*>(registry)->template Get<TComponent>(entity);
				},
				[](void* registry, const EntityId entity, TComponent component)
				{
					if constexpr (std::is_empty_v<TComponent>)
						return static_cast<TRegistry*>(registry)->template Assign<TComponent>(entity);
					else
						return static_cast<TRegistry*>(registry)->template Assign<TComponent>(entity,
						                                                                      std::move(component));
				},
				[](void* registry, const EntityId entity)
				{
					return static_cast<TRegistry*>(registry)->template Remove<TComponent>(entity);
				}
			};
		}

		template <typename TList>
		struct ComponentOperationTuple;

		template <template <typename...> typename TList, typename... TComponents>
		struct ComponentOperationTuple<TList<TComponents...>> final
		{
			using Type = std::tuple<ComponentOperations<TComponents>...>;

			template <Registry TRegistry>
			[[nodiscard]] static constexpr Type Bind() noexcept
			{
				return {BindComponentOperations<TComponents, TRegistry>()...};
			}
		};

		template <typename TQuery>
		struct QueryOperations;

		template <typename... TRead, typename... TWrite, typename... TWith, typename... TExclude>
		struct QueryOperations<Query<Read<TRead...>, Write<TWrite...>, With<TWith...>, Exclude<TExclude...>>> final
		{
			using QueryType = Query<Read<TRead...>, Write<TWrite...>, With<TWith...>, Exclude<TExclude...>>;
			using Callback = void (*)(void*, EntityId, const TRead&..., TWrite&...);
			using Execute = void (*)(void*, void*, Callback);

			Execute ForEach;

			template <typename TFunction>
			static void Invoke(void* state, const EntityId entity, const TRead&... read, TWrite&... write)
			{
				std::invoke(*static_cast<TFunction*>(state), entity, read..., write...);
			}

			template <Registry TRegistry>
			[[nodiscard]] static constexpr QueryOperations Bind() noexcept
			{
				return {
					[](void* registry, void* state, const Callback callback)
					{
						static_cast<TRegistry*>(registry)->ForEach(
							QueryType{},
							[state, callback](const EntityId entity, const TRead&... read, TWrite&... write)
							{
								callback(state, entity, read..., write...);
							});
					}
				};
			}
		};
	}

	template <typename T, typename TPermissions>
	concept Readable = Detail::ContainsType<T, typename TPermissions::ReadAccess> ||
		Detail::ContainsType<T, typename TPermissions::WriteAccess> ||
		Detail::ContainsType<T, typename TPermissions::StructuralAccess>;

	template <typename T, typename TPermissions>
	concept Writable = Detail::ContainsType<T, typename TPermissions::WriteAccess> ||
		Detail::ContainsType<T, typename TPermissions::StructuralAccess>;

	template <typename T, typename TPermissions>
	concept StructurallyWritable = Detail::ContainsType<T, typename TPermissions::StructuralAccess>;

	namespace Detail
	{
		template <typename TPermissions, typename TList>
		inline constexpr bool AllReadable = false;

		template <typename TPermissions, template <typename...> typename TList, typename... TComponents>
		inline constexpr bool AllReadable<TPermissions, TList<TComponents...>> =
			(Readable<TComponents, TPermissions> && ...);

		template <typename TPermissions, typename TList>
		inline constexpr bool AllWritable = false;

		template <typename TPermissions, template <typename...> typename TList, typename... TComponents>
		inline constexpr bool AllWritable<TPermissions, TList<TComponents...>> =
			(Writable<TComponents, TPermissions> && ...);

		template <typename TQuery, typename TPermissions>
		inline constexpr bool QueryPermitted = false;

		template <typename TRead, typename TWrite, typename TWith, typename TExclude, typename TPermissions>
		inline constexpr bool QueryPermitted<Query<TRead, TWrite, TWith, TExclude>, TPermissions> =
			AllReadable<TPermissions, TRead> && AllWritable<TPermissions, TWrite> && AllReadable<TPermissions, TWith> &&
			AllReadable<TPermissions, TExclude>;
	}

	template <typename TPermissions, typename... TQueries>
		requires(Detail::QueryPermitted<TQueries, TPermissions> && ...)
	class Access final
	{
		using ReadOperations = typename Detail::ComponentOperationTuple<typename TPermissions::ReadAccess>::Type;
		using WriteOperations = typename Detail::ComponentOperationTuple<typename TPermissions::WriteAccess>::Type;
		using StructuralOperations =
		typename Detail::ComponentOperationTuple<typename TPermissions::StructuralAccess>::Type;

	public:
		using PermissionSet = TPermissions;

		template <Registry TRegistry>
		explicit Access(TRegistry& registry) noexcept :
			m_Registry(&registry),
			m_Scope([](const void* value) noexcept { return static_cast<const TRegistry*>(value)->Scope(); }),
			m_IsAlive([](const void* value, const EntityId entity) noexcept
			{
				return static_cast<const TRegistry*>(value)->IsAlive(entity);
			}),
			m_Create([](void* value) { return static_cast<TRegistry*>(value)->Create(); }),
			m_Destroy([](void* value, const EntityId entity)
			{
				return static_cast<TRegistry*>(value)->Destroy(entity);
			}),
			m_ReadOperations(
				Detail::ComponentOperationTuple<typename TPermissions::ReadAccess>::template Bind<TRegistry>()),
			m_WriteOperations(
				Detail::ComponentOperationTuple<typename TPermissions::WriteAccess>::template Bind<TRegistry>()),
			m_StructuralOperations(
				Detail::ComponentOperationTuple<typename TPermissions::StructuralAccess>::template Bind<TRegistry>()),
			m_QueryOperations(Detail::QueryOperations<TQueries>::template Bind<TRegistry>()...)
		{
		}

		[[nodiscard]] RegistryScopeId Scope() const noexcept { return m_Scope(m_Registry); }

		[[nodiscard]] bool IsAlive(const EntityId entity) const noexcept { return m_IsAlive(m_Registry, entity); }

		template <typename TComponent>
			requires Readable<TComponent, TPermissions>
		[[nodiscard]] bool Contains(const EntityId entity) const noexcept
		{
			return Operations<TComponent>().Contains(m_Registry, entity);
		}

		template <typename TComponent>
			requires Readable<TComponent, TPermissions> && (!std::is_empty_v<TComponent>)
		[[nodiscard]] auto Get(const EntityId entity) const
		{
			return Operations<TComponent>().GetConst(m_Registry, entity);
		}

		template <typename TComponent>
			requires Writable<TComponent, TPermissions> && (!std::is_empty_v<TComponent>)
		[[nodiscard]] auto Get(const EntityId entity)
		{
			return Operations<TComponent>().GetMutable(m_Registry, entity);
		}

		template <typename TQuery, typename TFunction>
			requires(std::same_as<TQuery, TQueries> || ...)
		void ForEach(TQuery, TFunction&& function)
		{
			using Function = std::remove_reference_t<TFunction>;
			auto* state = const_cast<void*>(static_cast<const void*>(std::addressof(function)));
			std::get<Detail::QueryOperations<TQuery>>(m_QueryOperations)
				.ForEach(m_Registry, state, &Detail::QueryOperations<TQuery>::template Invoke<Function>);
		}

		template <typename TComponent>
			requires StructurallyWritable<TComponent, TPermissions> && (!std::is_empty_v<TComponent>)
		[[nodiscard]] auto Assign(const EntityId entity, TComponent component)
		{
			return Operations<TComponent>().Assign(m_Registry, entity, std::move(component));
		}

		template <typename TComponent>
			requires StructurallyWritable<TComponent, TPermissions> && std::is_empty_v<TComponent>
		[[nodiscard]] auto Assign(const EntityId entity)
		{
			return Operations<TComponent>().Assign(m_Registry, entity, TComponent{});
		}

		template <typename TComponent>
			requires StructurallyWritable<TComponent, TPermissions>
		[[nodiscard]] auto Remove(const EntityId entity)
		{
			return Operations<TComponent>().Remove(m_Registry, entity);
		}

		[[nodiscard]] EntityId Create()
			requires(TPermissions::Entities == EntityAuthority::Own)
		{
			return m_Create(m_Registry);
		}

		[[nodiscard]] auto Destroy(const EntityId entity)
			requires(TPermissions::Entities == EntityAuthority::Own)
		{
			return m_Destroy(m_Registry, entity);
		}

	private:
		template <typename TComponent>
		[[nodiscard]] const Detail::ComponentOperations<TComponent>& Operations() const noexcept
		{
			if constexpr (Detail::ContainsType<TComponent, typename TPermissions::StructuralAccess>)
				return std::get<Detail::ComponentOperations<TComponent>>(m_StructuralOperations);
			else if constexpr (Detail::ContainsType<TComponent, typename TPermissions::WriteAccess>)
				return std::get<Detail::ComponentOperations<TComponent>>(m_WriteOperations);
			else
				return std::get<Detail::ComponentOperations<TComponent>>(m_ReadOperations);
		}

		void* m_Registry;
		RegistryScopeId (*m_Scope)(const void*) noexcept;
		bool (*m_IsAlive)(const void*, EntityId) noexcept;
		EntityId (*m_Create)(void*);
		std::expected<void, EntityOperationError> (*m_Destroy)(void*, EntityId);
		ReadOperations m_ReadOperations;
		WriteOperations m_WriteOperations;
		StructuralOperations m_StructuralOperations;
		std::tuple<Detail::QueryOperations<TQueries>...> m_QueryOperations;
	};
}
