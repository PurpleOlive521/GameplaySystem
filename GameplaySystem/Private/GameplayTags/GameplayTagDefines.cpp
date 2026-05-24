// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTagDefines.h"

/*
* The order of declaration is important, since parent tag comments are only preserved if declared first.
* The child tags will overwrite all parent tag comments if declared first.
*/

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes,				"Modes", "Designates ownership of a mode, or associates the object with that mode.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Normal,		"Modes.Normal", "Normal mode.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Infrared,		"Modes.Infrared", "Infared mode.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Ultraviolet,	"Modes.Ultraviolet", "Ultraviolet mode.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Modes_Gamma,			"Modes.Gamma", "Gamma mode.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player,					"Player", "Tags unique to the player.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_ShowCombatHud,			"Player.ShowCombatHud",	"Prompts the entire Hud to be shown, including combat elements.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AggroedEnemy,				"Player.AggroedEnemy", "The amount of enemies currently aggroed on the player.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_TargetLockSwitchDisabled, "Player.TargetLockSwitchDisabled", "Can't switch target while applied.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_Combo,					"Player.Combo", "The combo action we are currently performing.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_Combo_DefaultCount,		"Player.Combo.DefaultCount", "Default combo counter.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_Combo_HeavyCount,			"Player.Combo.HeavyCount", "Heavy combo counter");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_AirDashed,				"Player.AirDashed", "An air dash has recently been performed.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Player_ChargingAttack,			"Player.ChargingAttack", "Charging up the Charge Attack");


UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Enemy,	"Enemy", "Tags unique to enemies.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Enemy_Status, "Enemy.Status", "Specifiers for different ailments and states that only Enemies can gain or inflict.")

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Enemy_Status_Awakened,				"Enemy.Status.Awakened", "The Enemy has activated from a previously passive state, usually with a unique one-time animation.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Enemy_Status_PhaseCount,				"Enemy.Status.PhaseCount", "Which phase the Enemy is in. The tag count indicates phase count, where the current phase is tag count + 1.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Enemy_Status_EquippedWeapon,			"Enemy.Status.EquippedWeapon", "Enemy has their weapon equipped.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Enemy_Status_StrafingAroundTarget,	"Enemy.Status.StrafingAroundTarget", "Enemy is strafing around a target.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Enemy_Status_StrongEnemy,			"Enemy.Status.StrongEnemy", "The Enemy is strong relative to other opponents, usually considered a mini-boss or boss.");


UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status, "Status", "Specifiers for different ailments and states that entities can gain or inflict.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_BufferingAction,			"Status.BufferingAction", "Some actions when requested are buffered if currently not able to be performed.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_PerformingAction,			"Status.PerformingAction", "The character is performing an action that prohibits other actions from being started until finished.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_CanPerfectDodge,			"Status.CanPerfectDodge", "Allows a perfect dodge to be triggered if the character's mode is the same as that of an incoming attack.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_HyperArmor,				"Status.HyperArmor", "The character's actions are uninterruptable, but it can still take damage.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Invincible,				"Status.Invincible", "The character's actions are interruptable, but it can't be damaged.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_IFrames,					"Status.IFrames", "Similar in function as Invincible, except that certain actions are permitted to deal damage through IFrames, such as AOE or DOT.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Untargetable,				"Status.Untargetable", "The character's actions are uninterruptable, and it can't be damaged.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Overheating,				"Status.Overheating", "Character will overheat in a short bit, causing damage if not recovered from.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_OverheatBuildupImmunity,	"Status.OverheatBuildupImmunity", "Character will not receive Overheat buildup from sources deemed negative.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Overclocking,				"Status.Overclocking", "The character is within the OverclockThreshold and is temporarily buffed.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_CombatReady,				"Status.CombatReady", "The character is in a hostile environment or ready for combat, but not necessarily in combat yet.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_CombatReady_InCombat,		"Status.CombatReady.InCombat", "The character is in combat.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Dying,					"Status.Dying", "The character is dead but is playing some GameplayAbility, animation or similar before dying.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Dead,						"Status.Dead",	"The character has played it's on-death effects and actions and is now fully dead.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Running,					"Status.Running", "The character is running, which increases the movement speed.")
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Dashing,					"Status.Dashing", "The character is dashing.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_KnockbackImmunity,		"Status.KnockbackImmunity", "The character can not receive knockback.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Status_Aiming,					"Status.Aiming", "The character is aiming at something.");

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

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_Death, "AbilitySlot.Death", "");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AbilitySlot_PerfectParryResponse, "AbilitySlot.PerfectParryResponse", "An action to perform in response to BEING perfect parried. Should often override other active actions.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility,			"GameplayAbility", "Tags meant for GameplayAbilities.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types,	"GameplayAbility.Types", "Specifiers meant to distinguish between different Abilities based on their features and purpose.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_BlockingAction, "GameplayAbility.Types.BlockingAction", "Should, by design, block other BlockingActions when activated and be itself blocked by them");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_Dash,			"GameplayAbility.Types.Dash", "The ability is an movement-based action that performs a dash.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_Overcharge,	"GameplayAbility.Types.Overcharge", "Part of the overcharging sequence.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_Overheat,		"GameplayAbility.Types.Overheat", "In response to a Overheat.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_Special,		"GameplayAbility.Types.Special", "The action is a special, e.g. a special move of some kind");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_ModeSwitch,	"GameplayAbility.Types.ModeSwitch", "The action will switch mode of the character");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Types_Jump,			"GameplayAbility.Types.Jump", "The action is a movement-based action that performs a jump.");


UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Status,	"GameplayAbility.Status", "States that GameplayAbilities can have.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Status_ActionCancellable,		"GameplayAbility.Status.ActionCancellable", "A activating action can choose to cancel this one.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayAbility_Status_MovementCancellable,		"GameplayAbility.Status.MovementCancellable", "This wants to be cancelled when movement-input is received.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEffect, "GameplayEffect", "Tags meant for GameplayEffects.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEffect_Types, "GameplayEffect.Types", "Specifiers meant to distinguish between different GameplayEffects based on their features and purpose.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEffect_Types_Overheat, "GameplayEffect.Types.Overheat", "Applies Overheat to the associated Mode.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEffect_Triggers,			"GameplayEffect.Triggers", "Different activators for GameplayEffects, usually performed through removal of the GameplayEffect.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEffect_Triggers_HeavyAttack, "GameplayEffect.Triggers.HeavyAttack", "Trigger of type HeavyAttack.");


UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent, "GameplayEvent", "Tags meant for GameplayEvents.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Types, "GameplayEvent.Types", "Specifiers meant to distinguish between different GameplayEvents based on their features and purpose.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Types_Expression,		"GameplayEvent.Types.Expression", "Modifies or sets the Expression material of a Character.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Types_TimeSlow, 		"GameplayEvent.Types.TimeSlow", "Derives from GE_TimeSlow");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Types_TimeSlow_HitStop,	"GameplayEvent.Types.TimeSlow.HitStop", "Variations of hitstop.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Effects,	"GameplayEvent.Effects", "Different effects that the event applies. Allows for disabling certain events, or override their effects.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Effects_Material, "GameplayEvent.Effects.Materials", "Affects the Material.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Effects_OverlayMaterial, "GameplayEvent.Effects.OverlayMaterial", "Affects the Overlay Material.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_GameplayEvent_Effects_TimeDilation, "GameplayEvent.Effects.TimeDilation", "Affects TimeDilation.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AI, "AI", "AI related tags, often used in BehaviourTrees or Blackboards.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AI_Actions, "AI.Actions", "Different AI actions.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_AI_Actions_AttackSequence, "AI.Actions.AttackSequence", "A sequence or combo of attack actions.");



UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Hitboxes, "Hitboxes", "Tags that apply to hitboxes.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Hitboxes_Types, "Hitboxes.Types", "Differentiates different kinds of hitboxes based on purpose or source.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Hitboxes_Types_Weapon,	"Hitboxes.Types.Weapon", "The hitbox originates or encompasses a weapon that requires physical overlap.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Hitboxes_Types_AOE,		"Hitboxes.Types.AOE", "The hitbox is a form of area-of-effect, and does not need direct contact with instigator object to overlap.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Hitboxes_Contexts,		"Hitboxes.Contexts", "Additional context to give to hitboxes.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAYTAG_Hitboxes_Contexts_HeavyAttack,	"Hitboxes.Contexts.HeavyAttack", "Triggered by an attack classified as a HeavyAttack.");
