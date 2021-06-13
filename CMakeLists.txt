set (ECS_SRCS
	src/ecs/Manager.cpp
	src/ecs/System.cpp
	src/ecs/cache/ComponentsTuple.cpp
	src/ecs/cache/ComponentsTupleCache.cpp
	src/ecs/component/ComponentPtr.cpp
	src/ecs/detail/ComponentCollectionManagerConnection.cpp
	src/ecs/detail/Hash.cpp
	src/ecs/entity/EntitiesCollection.cpp
	src/ecs/entity/Entity.cpp
	src/ecs/entity/EntityData.cpp
	src/ecs/entity/EntityLayer.cpp)

add_library(raven_ecs SHARED ${ECS_SRCS})

include(GenerateExportHeader)
generate_export_header(raven_ecs
	EXPORT_MACRO_NAME ECS_API)

target_include_directories(raven_ecs
	PUBLIC
		src
		${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(raven_ecs raven_events)
