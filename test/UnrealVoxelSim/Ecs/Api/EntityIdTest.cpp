#include "UnrealVoxelSim/Ecs/Api/EntityId.h"
#include "UnrealVoxelSim/Ecs/Api/Detail/EntityIdAccess.h"
#include "UnrealVoxelSim/Ecs/Api/Query.h"
#include "UnrealVoxelSim/Ecs/Api/RegistryScopeId.h"

#include <gtest/gtest.h>

namespace UnrealVoxelSim::Ecs::Api
{

TEST(EntityIdTest, DefaultIdentifierIsInvalid)
{
    EXPECT_FALSE(EntityId{}.IsValid());
}

TEST(EntityIdTest, RetainsRegistryScopeAndLocalValue)
{
    constexpr RegistryScopeId Scope{17};
    constexpr auto Entity = Detail::EntityIdAccess::Create(Scope, 42);

    static_assert(Entity.IsValid());
    static_assert(Entity.Scope() == Scope);
    static_assert(Detail::EntityIdAccess::LocalValue(Entity) == 42);

    EXPECT_EQ(Entity.Scope(), Scope);
    EXPECT_EQ(Detail::EntityIdAccess::LocalValue(Entity), 42U);
}

TEST(EntityIdTest, QueryDescriptorPreservesAccessCategories)
{
    struct Position
    {
    };
    struct Velocity
    {
    };
    struct Disabled
    {
    };

    using Descriptor = Query<Read<Position>, Write<Velocity>, With<>, Exclude<Disabled>>;

    static_assert(std::same_as<typename Descriptor::ReadAccess, Read<Position>>);
    static_assert(std::same_as<typename Descriptor::WriteAccess, Write<Velocity>>);
    static_assert(std::same_as<typename Descriptor::Excluded, Exclude<Disabled>>);
}

}
