#pragma once

namespace UnrealVoxelSim::Ecs::Api
{
	enum class ComponentOperationError
	{
		EntityNotAlive,
		AlreadyPresent,
		NotPresent,
	};
}
