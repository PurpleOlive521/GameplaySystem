// Copyright (c) 2026, Heavy Duty Tape Studios. All rights reserved.

using UnrealBuildTool;

public class GameplaySystemEditor: ModuleRules
{
    public GameplaySystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {
                            "Core", 
                            "CoreUObject", 
                            "Engine", 
                            "GameplaySystem", 
                            "Slate",   
                            "SlateCore",
                            "AssetTools",
                            "ClassViewer",
                            "GameplayTags",
                            "GameplayTagsEditor",
                            "InputCore",
                            "PropertyEditor",
                            "BlueprintGraph",
                            "Kismet",
                            "KismetCompiler",
                            "GraphEditor",
                            "LevelSequence",
                            "MainFrame",
                            "EditorFramework",
                            "UnrealEd",
                            "WorkspaceMenuStructure",
                            "ContentBrowser",
                            "EditorWidgets",
                            "SourceControl",
                            "ToolMenus"
        });
    }
}