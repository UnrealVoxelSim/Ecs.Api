# UnrealVoxelSim.Ecs.Api

Backend-independent, header-only compile-time contracts for entity-component storage.

`EntityId` is an opaque, generation-aware runtime identifier scoped to one registry. Its representation is not a save
format and must not be persisted. Ordinary checked component access returns `std::expected`; query callbacks receive
direct references because the query establishes component validity.

Queries declare read, write, required-filter, and excluded-filter components explicitly. Structural changes during
iteration are deferred through the selected backend's command buffer. Registries are thread-affine unless their backend
documents a stronger contract.

Registry objects are composition-owned infrastructure. Dynamic system APIs do not accept registries, views, component
storage, or command buffers.
