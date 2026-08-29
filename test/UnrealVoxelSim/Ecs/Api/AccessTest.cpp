#include "UnrealVoxelSim/Ecs/Api/Access.h"
#include "UnrealVoxelSim/Ecs/Api/ComponentOperationError.h"
#include "UnrealVoxelSim/Ecs/Api/Detail/EntityIdAccess.h"

#include <gtest/gtest.h>

#include <expected>
#include <functional>
#include <utility>

namespace UnrealVoxelSim::Ecs::Api
{
	namespace
	{
		struct PositionComponent final
		{
			int Value{};
		};

		struct VelocityComponent final
		{
			int Value{};
		};

		struct OwnedComponent final
		{
			int Value{};
		};

		class FakeRegistry final
		{
		public:
			struct CommandBuffer final
			{
			};

			struct CommitResult final
			{
			};

			[[nodiscard]] RegistryScopeId Scope() const noexcept { return RegistryScopeId{1}; }

			[[nodiscard]] EntityId Create() { return Detail::EntityIdAccess::Create(Scope(), 1); }

			[[nodiscard]] bool IsAlive(EntityId) const noexcept { return true; }

			[[nodiscard]] std::expected<void, EntityOperationError> Destroy(EntityId) { return {}; }

			[[nodiscard]] CommandBuffer MakeCommandBuffer() { return {}; }

			[[nodiscard]] CommitResult Commit(CommandBuffer&) { return {}; }

			template <typename TComponent>
			[[nodiscard]] bool Contains(EntityId) const noexcept
			{
				return true;
			}

			template <typename TComponent>
			[[nodiscard]] std::expected<std::reference_wrapper<TComponent>, ComponentOperationError> Get(EntityId)
			{
				return std::ref(Component<TComponent>());
			}

			template <typename TComponent>
			[[nodiscard]] std::expected<std::reference_wrapper<const TComponent>, ComponentOperationError>
			Get(EntityId) const
			{
				return std::cref(Component<TComponent>());
			}

			template <typename TComponent, typename... TArguments>
			[[nodiscard]] std::expected<void, ComponentOperationError> Emplace(EntityId, TArguments&&...)
			{
				return {};
			}

			template <typename TComponent, typename... TArguments>
			[[nodiscard]] std::expected<void, ComponentOperationError> Assign(EntityId, TArguments&&...)
			{
				return {};
			}

			template <typename TComponent>
			[[nodiscard]] std::expected<void, ComponentOperationError> Remove(EntityId)
			{
				return {};
			}

			template <typename TQuery, typename TFunction>
			void ForEach(TQuery, TFunction&& function)
			{
				std::invoke(std::forward<TFunction>(function),
							Detail::EntityIdAccess::Create(Scope(), 1),
							std::as_const(m_Position),
							m_Velocity);
			}

		private:
			template <typename TComponent>
			[[nodiscard]] TComponent& Component()
			{
				if constexpr (std::same_as<TComponent, PositionComponent>)
					return m_Position;
				else if constexpr (std::same_as<TComponent, VelocityComponent>)
					return m_Velocity;
				else
					return m_Owned;
			}

			template <typename TComponent>
			[[nodiscard]] const TComponent& Component() const
			{
				if constexpr (std::same_as<TComponent, PositionComponent>)
					return m_Position;
				else if constexpr (std::same_as<TComponent, VelocityComponent>)
					return m_Velocity;
				else
					return m_Owned;
			}

			PositionComponent m_Position{2};
			VelocityComponent m_Velocity{3};
			OwnedComponent m_Owned{4};
		};

		static_assert(Registry<FakeRegistry>);

		using TestPermissions =
			Permissions<Read<PositionComponent>, Write<VelocityComponent>, Structural<OwnedComponent>>;
		using TestQuery = Query<Read<PositionComponent>, Write<VelocityComponent>>;
		using TestAccess = Access<TestPermissions, TestQuery>;
		using ForbiddenQuery = Query<Read<>, Write<PositionComponent>>;

		template <typename TPermissions, typename TQuery>
		concept DefinesAccess = requires { typename Access<TPermissions, TQuery>; };

		template <typename TAccess>
		concept MutablePositionAccess = requires(TAccess access, EntityId entity) {
			{
				access.template Get<PositionComponent>(entity)
			} -> std::same_as<std::expected<std::reference_wrapper<PositionComponent>, ComponentOperationError>>;
		};

		template <typename TAccess>
		concept PositionRemovalAccess =
			requires(TAccess access, EntityId entity) { access.template Remove<PositionComponent>(entity); };

		template <typename TAccess>
		concept OwnedRemovalAccess =
			requires(TAccess access, EntityId entity) { access.template Remove<OwnedComponent>(entity); };

		static_assert(!MutablePositionAccess<TestAccess>);
		static_assert(!PositionRemovalAccess<TestAccess>);
		static_assert(OwnedRemovalAccess<TestAccess>);
		static_assert(!DefinesAccess<TestPermissions, ForbiddenQuery>);

		TEST(AccessTest, QueryProvidesOnlyDeclaredReferenceKinds)
		{
			FakeRegistry registry;
			TestAccess access{registry};

			access.ForEach(TestQuery{},
						   [](EntityId, const PositionComponent& position, VelocityComponent& velocity)
						   { velocity.Value += position.Value; });

			EXPECT_EQ(access.Get<VelocityComponent>({})->get().Value, 5);
			static_cast<void>(access.Assign<OwnedComponent>({}, OwnedComponent{7}));
		}
	}
}
