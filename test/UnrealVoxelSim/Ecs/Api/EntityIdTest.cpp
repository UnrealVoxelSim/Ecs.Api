#include "UnrealVoxelSim/Ecs/Api/EntityId.h"
#include "UnrealVoxelSim/Ecs/Api/Backend/EntityIdAccess.h"
#include "UnrealVoxelSim/Ecs/Api/Query.h"
#include "UnrealVoxelSim/Ecs/Api/RegistryScopeId.h"

#include <gtest/gtest.h>

namespace UnrealVoxelSim::Ecs::Api
{
	TEST(EntityIdTest, DefaultIdentifierIsInvalid) { EXPECT_FALSE(EntityId{}.IsValid()); }

	TEST(EntityIdTest, RetainsRegistryScopeAndBackendValue)
	{
		constexpr RegistryScopeId Scope{17};
		constexpr auto Entity = Backend::EntityIdAccess::Create(Scope, 42);

		static_assert(Entity.IsValid());
		static_assert(Entity.Scope() == Scope);
		static_assert(Backend::EntityIdAccess::GetValue(Entity) == 42);

		EXPECT_EQ(Entity.Scope(), Scope);
		EXPECT_EQ(Backend::EntityIdAccess::GetValue(Entity), 42U);
	}

	TEST(EntityIdTest, QueryDescriptorPreservesAccessCategories)
	{
		struct PositionComponent
		{
		};
		struct VelocityComponent
		{
		};
		struct DisabledComponent
		{
		};

		using Descriptor = Query<Read<PositionComponent>, Write<VelocityComponent>, With<>, Exclude<DisabledComponent>>;

		static_assert(std::same_as<typename Descriptor::ReadAccess, Read<PositionComponent>>);
		static_assert(std::same_as<typename Descriptor::WriteAccess, Write<VelocityComponent>>);
		static_assert(std::same_as<typename Descriptor::Excluded, Exclude<DisabledComponent>>);
	}
}
