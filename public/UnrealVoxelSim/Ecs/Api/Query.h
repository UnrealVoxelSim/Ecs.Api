#pragma once

namespace UnrealVoxelSim::Ecs::Api
{

template <typename... TComponents> struct Read final
{
};

template <typename... TComponents> struct Write final
{
};

template <typename... TComponents> struct With final
{
};

template <typename... TComponents> struct Exclude final
{
};

template <typename TRead, typename TWrite = Write<>, typename TWith = With<>, typename TExclude = Exclude<>>
struct Query final
{
    using ReadAccess = TRead;
    using WriteAccess = TWrite;
    using Required = TWith;
    using Excluded = TExclude;
};

} // namespace UnrealVoxelSim::Ecs::Api
