// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "GameplayTagDefines.h"

/*
* The order of declaration is important, since parent tag comments are only preserved if declared first.
* The child tags will overwrite all parent tag comments if declared first.
*/

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes,				"Modes", "Designates which mode an entity is in, and by extension what type of damage they deal.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Normal,		"Modes.Normal", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Infrared,		"Modes.Infrared", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Ultraviolet,	"Modes.Ultraviolet", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Gamma,			"Modes.Gamma", "");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player,					"Player", "Tags unique to the player.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_ShowCombatHud,	"Player.ShowCombatHud",	"Prompts the entire Hud to be shown, including combat elements.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AggroedEnemy,	"Player.AggroedEnemy", "The amount of enemies currently aggroed on the player.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AvailableModes,	"Player.AvailableModes", "Designates if the mode is possible to switch to for the player.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AvailableModes_Normal,		"Player.AvailableModes.Normal", "Player has Normal.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AvailableModes_Infrared,		"Player.AvailableModes.Infrared", "Player has Infrared.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AvailableModes_Ultraviolet,	"Player.AvailableModes.Ultraviolet", "Player has Ultraviolet.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AvailableModes_Gamma,			"Player.AvailableModes.Gamma", "Player has Gamma.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Enemy,	"Enemy", "Tags unique to enemies.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Enemy_EquippedWeapon,		"Enemy.EquippedWeapon", "Enemy has their weapon equipped.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Enemy_StrafingAroundTarget,	"Enemy.StrafingAroundTarget", "Enemy is strafing around a target.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status, "Status", "Specifiers for different ailments and states that entities can gain or inflict.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_BufferingAction,			"Status.BufferingAction", "Some actions when requested are buffered if currently not able to be performed.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_PerformingAction,			"Status.PerformingAction", "The character is performing an action that prohibits other actions from being started until finished.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_CanPerfectDodge,			"Status.CanPerfectDodge", "Allows a perfect dodge to be triggered if the character's mode is the same as that of an incoming attack.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_HyperArmor,				"Status.HyperArmor", "The character's actions are uninterruptable, but it can still take damage.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Invincible,				"Status.Invincible", "The character's actions are interruptable, but it can't be damaged.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_IFrames,					"Status.IFrames", "Similar in function as Invincible, except that certain actions are permitted to deal damage through IFrames, such as AOE or DOT.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Untargetable,				"Status.Untargetable", "The character's actions are uninterruptable, and it can't be damaged.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_RecentlyPerformedAction,	"Status.RecentlyPerformedAction", "A action was recently performed. Usually means the character is mid-combat.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Overheating,				"Status.Overheating", "Character will overheat in a short bit, causing damage if not recovered from.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_InCombat,					"Status.InCombat", "The character is currently in combat.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot, "AbilitySlot", "");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_ModeSwitch, "AbilitySlot.ModeSwitch", "");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_ModeSwitch_Normal, "AbilitySlot.ModeSwitch.Normal", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_ModeSwitch_Infrared, "AbilitySlot.ModeSwitch.Infrared", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_ModeSwitch_Ultraviolet, "AbilitySlot.ModeSwitch.Ultraviolet", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_ModeSwitch_Gamma, "AbilitySlot.ModeSwitch.Gamma", "");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_Ability, "AbilitySlot.Ability", "");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_Ability_Normal, "AbilitySlot.Ability.Normal", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_Ability_Infrared, "AbilitySlot.Ability.Infrared", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_Ability_Ultraviolet, "AbilitySlot.Ability.Ultraviolet", "");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_Ability_Gamma, "AbilitySlot.Ability.Gamma", "");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility,			"GameplayAbility", "Tags meant for GameplayAbilities.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types,	"GameplayAbility.Types", "Specifiers meant to distinguish between different Abilities based on their features and purpose.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_PrimaryAction, "GameplayAbility.Types.PrimaryAction", "Signifies that the Ability prohibits further actions unless cancelled, or wants to be treated as such.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_Dash,			"GameplayAbility.Types.Dash", "The ability is an movement-based action of the type 'Dash'.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_Overcharge,	"GameplayAbility.Types.Overcharge", "Part of the overcharging sequence.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Status,	"GameplayAbility.Status", "States that GameplayAbilities can have.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Status_PrimaryActionCancellable, "GameplayAbility.Status.PrimaryActionCancellable", "This wants to be cancelled by any PrimaryActions that activate.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Status_MovementCancellable,		"GameplayAbility.Status.MovementCancellable", "This wants to be cancelled when movement-input is received.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEffect, "GameplayEffect", "Tags meant for GameplayEffects.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEffect_Types, "GameplayEffect.Types", "Specifiers meant to distinguish between different GameplayEffects based on their features and purpose.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEffect_Types_OverheatTick, "GameplayEffect.Types.OverheatTick", "Applies Overheat to the associated Mode.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent, "GameplayEvent", "Tags meant for GameplayEvents.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Types, "GameplayEvent.Types", "Specifiers meant to distinguish between different GameplayEvents based on their features and purpose.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Types_Expression,		"GameplayEvent.Types.Expression", "Modifies or sets the Expression material of a Character.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Effects,	"GameplayEvent.Effects", "Different effects that the event applies. Allows for disabling certain events, or override their effects.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Effects_Material, "GameplayEvent.Effects.Materials", "Affects the Material.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Effects_OverlayMaterial, "GameplayEvent.Effects.OverlayMaterial", "Affects the Overlay Material.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Effects_TimeDilation, "GameplayEvent.Effects.TimeDilation", "Affects TimeDilation.");