###### Gameplay System

A modular gameplay module featuring a standalone GameplaySystem component, allowing any Actor to posses activatable abilities, gameplay attributes \& tags and temporary effects. 



Designed around derivable superclasses that allow designers to prototype \& create gameplay on their own.



###### Features:

* GameplayAbilities - Activatable abilities that can trigger additional actions and logic. Supports cooldowns \& duration, activation and cancellation dependencies and requirements, and complex cancellation effect chains. Fully derivable in both C++ and blueprint.



* GameplayEffects - Self-contained temporary modifiers that has an duration attached. Can be used to modify stats or apply \& remove GameplayTags. Supports reactivation and damage-over-time effects. Derivable in both C++ and blueprint.



* Attributes - Stats that can be modified externally and used to drive gameplay. Can be managed by a level system to increase over gameplay time, and support complex fractional and percentage-based modifiers, while only being recalculated when necessary.



* GameplayTags - Tracks the total of each applied GameplayTag, allowing multiple sources to contribute to the removal or application of a GameplayTag. 



* Debugging suite - A widget that allows testers and designers to see information about the state of any GameplaySystem and it's attributes, abilities, modifiers and GameplayTags. Can be cycled to check any Actor currently in the Level.

&nbsp;

Done fully in C++ while allowing blueprint connectivity for key elements.



Inspired by Unreal Engines GameplayAbilityComponent. 

