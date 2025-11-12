// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "GameplayTagDefines.h"

/*
* The order of declaration is important, since parent tag comments are only preserved if declared first.
* The child tags will overwrite all parent tag comments if declared first.
*/

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes, "Modes", "Designates which mode an entity is in, and by extension what type of damage they deal.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Normal, "Modes.Normal", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Infrared, "Modes.Infrared", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Ultraviolet, "Modes.Ultraviolet", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Gamma, "Modes.Gamma", "");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player, "Player", "Tags unique to the player.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AvailableModes, "Player.AvailableModes", "Designates if the mode is possible to switch to for the player.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AvailableModes_Normal, "Player.AvailableModes.Normal", "Player has Normal.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AvailableModes_Infrared, "Player.AvailableModes.Infrared", "Player has Infrared.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AvailableModes_Ultraviolet, "Player.AvailableModes.Ultraviolet", "Player has Ultraviolet.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AvailableModes_Gamma, "Player.AvailableModes.Gamma", "Player has Gamma.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status, "Status", "Specifiers for different ailments and states that entities can gain or inflict.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_BufferingAction, "Status.BufferingAction", "Some actions when requested are buffered if currently not able to be performed.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_PerformingAction, "Status.PerformingAction", "The character is performing an action that prohibits other actions from being started until finished.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_CanPerfectDodge, "Status.CanPerfectDodge", "Allows a perfect dodge to be triggered if the character's mode is the same as that of an incoming attack.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_HyperArmor, "Status.HyperArmor", "The character's actions are uninterruptable, but it can still take damage.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Invincible, "Status.Invincible", "The character's actions are interruptable, but it can't be damaged.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_IFrames, "Status.IFrames", "Similar in function as Invincible, except that certain actions are permitted to deal damage through IFrames, such as AOE or DOT.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Untargetable, "Status.Untargetable", "The character's actions are uninterruptable, and it can't be damaged.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_RecentlyPerformedAction, "Status.RecentlyPerformedAction", "A action was recently performed. Usually means the character is mid-combat.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility, "GameplayAbility", "Tags meant for GameplayAbilities, to specify behaviour, features or types.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types, "GameplayAbility.Types", "Specifiers meant to distinguish between different Abilities based on their features and purpose.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_PrimaryAction, "GameplayAbility.Types.PrimaryAction", "Signifies that the Ability prohibits further actions unless cancelled, or wants to be treated as such.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_Dash, "GameplayAbility.Types.Dash", "The ability is an movement-based action of the type 'Dash'.");
