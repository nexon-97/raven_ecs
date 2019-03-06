# Overview #

This project implements **[Entity-Component-System](https://en.wikipedia.org/wiki/Entity_component_system)** management infrastructure for private game engine internal usage.

It doesn't contain any logic for entities and components [serialization](https://en.wikipedia.org/wiki/Serialization), because it is optional feature, and is implemented inside main engine project.

## Project structure ##

Solution comprises from several projects:

* **[ECS management project](docs/ECS_Overview.md)**
* Benchmarking project, based on SDL2 library for rendering purposes.
* Unit test project
