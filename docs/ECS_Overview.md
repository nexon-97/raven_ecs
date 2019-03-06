# Entity-Component-System project overview #

ECS behavior is contained inside [Manager](/src/ecs/Manager.hpp) class.

## Manager ##

It contains:

* Component collections (one collection per each component type)
* Entities collection (entities data storage)
* Systems collection

## Entities collection ##

Entities collection contains a pool of entities data.
Each entity has no logic, it only knows about:

* Its own id
* Parent id
* Types of components it has (Components association is defined by bitmask)
* Pointers to entity hierarchy data (which contains information about direct children of each entity, designed as linked lists mixture inside single storage vector)
* Pointers to entity components handles data (which contains information about entity components, by using component handles, which identify component type and id of the component in its collection, designed as linked lists mixture inside single storage vector)
* Pointers to entities are never invalidated (reallocation never happens inside collection), so you can safely store entities pointers wherever you like
* All collection internal data arrays are self-defragmentated, which means there are no holes in data, it is always filled immediately when the object is destroyed in any part of the storage

#### Entity structure ####

* entity id - **4 bytes**
* parent id - **4 bytes**
* components mask - **4 bytes**
* components data offset - **4 bytes**
* children data offset - **4 bytes**

Total - **20 bytes**.

##### Entity child mapping #####

* child id - **4 bytes**
* next entry pointer - **4 bytes**

Total - **8 bytes**.

#### Entity component mapping ####

* component handle - **4 bytes**
* next entry pointer - **4 bytes**

Total - **8 bytes**.

_Each entity has at least one entry for child and component mappings, so minimum entity size in ECS system is **36 bytes**_.

#### Component handle structure ####

* component type - **1 bytes** (only 32 types of components are supported, because of the components mask inside entity)
* component id - **3 bytes** (16.7 million components of each types supported at a time)
