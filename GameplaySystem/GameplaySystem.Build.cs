using System.IO;
using UnrealBuildTool;

public class GameplaySystem: ModuleRules
{
        public GameplaySystem(ReadOnlyTargetRules Target) : base(Target)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[] 
                {
                    "Core",
                    "CoreUObject", 
                    "Engine",

                    // Module-specific additions:
                    "EnhancedInput",
                    "CommonInput", // For accessing base input classes, like InputMapping
                    "GameplayTags",
                    "UMG", // Widget functionality in native

                }
                );

            // Editor only dependencies
            if(Target.bBuildEditor == true)
            {
                PublicDependencyModuleNames.AddRange( new string[] { "EditorTests"});
            }

            IncludeAllSubDirectories();
        }   

        // Adds all subdirectories under Source/ProjectName
        private void IncludeAllSubDirectories()
        {
            AddDirectoriesRecursive(ModuleDirectory);
        }

        // Recursively finds and adds all subdirectories, starting from Source/ProjectName
        private void AddDirectoriesRecursive(string DirectoryPathToSearch)
        {
            foreach (string DirectoryPath in Directory.GetDirectories(DirectoryPathToSearch))
            {
                PublicIncludePaths.Add(DirectoryPath);
                AddDirectoriesRecursive(DirectoryPath);
            }
        }
}