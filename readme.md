###### Gameplay System

A modular gameplay module featuring a standalone GameplaySystem component, allowing any Actor to posses activatable abilities, gameplay attributes \& tags and temporary effects. 

The GameplayEventSubsystem allows GameplayEvents to be instantiated without need for a owning Actor, and are managed in tandem with Unreal's Garbage Collection.

Also features QOL such as UObject-based Curve evaluators, a custom save system and more.



Designed around derivable superclasses that allow designers to prototype \& create gameplay on their own.

Uses GameplayTasks to drive asynchronous and latent work, which provides a intuitive control flow in Blueprint and modularizes key logic, without need for complex inheritance solutions.



###### Features:

* GameplayAbilities - Activatable abilities that can trigger additional actions and logic. Supports cooldowns \& duration, activation and cancellation dependencies and requirements, and complex cancellation effect chains. Fully derivable in both C++ and blueprint.



* GameplayEvents - Assetable events that can be triggered and instantiated during gameplay. Allows for complex VFX rulings and interactions with existing game feel elements, or conditional activation of new ones.



* GameplayEffects - Self-contained temporary modifiers that has an duration attached. Can be used to modify stats or apply \& remove GameplayTags. Supports reactivation and damage-over-time effects. Derivable in both C++ and blueprint.



* Attributes - Stats that can be modified externally and used to drive gameplay. Can be managed by a level system to increase over gameplay time, and support complex fractional and percentage-based modifiers, while only being recalculated when necessary.



* GameplayTags - Tracks the total of each applied GameplayTag, allowing multiple sources to contribute to the removal or application of a GameplayTag.



* Tasks - Latent tasks that perform work that exist within the scope of GameplayAbilities and GameplayEvents, making latent logic easy to implement and design around in Blueprint, without needing to resort to inheritance chains.



* GameplayPersistence - A autonomous save system that supports serializing Sublevels. Uses FArchives for context sensitive serialization and file compression. Automatically serializes destroyed Actors and SaveGame-tagged properties.



* Debugging suite - A widget that allows testers and designers to see information about the state of any GameplaySystem and it's attributes, abilities, modifiers and GameplayTags. Can be cycled to check any Actor currently in the Level, and supports multiple pages for viewing active GameplayEvents, and custom pages per project.

 

Done fully in C++ while allowing blueprint connectivity for key elements.



Inspired by Unreal Engine's GameplayAbilityComponent and Alex Stevens talk on their SaveGame module from Unreal Fest Gold Coast 2023.

