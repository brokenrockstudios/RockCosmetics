# 🪨 RockCosmetics 🪨

A community plugin version for handling cosmetic customization in Unreal Engine, derived from the Epic's Lyra Sample project, 
with support for Mutable (CustomizableObject).

## 📖 Overview

RockCosmetics is designed to manage character cosmetic and modular parts (e.g. hats, shirts, pants, etc.) for pawn customization, 
primarily for main playable characters.

Since this plugin is inspired by and forked from Lyra's Character Cosmetic system, reviewing the original documentation for the Lyra project is recommended.
However, this system is modular, meaning it can be introduced later in development without major integration concerns.

## 🛠️ Usage

There are two primary ways to use this RockCosmetics. 

1. **Vanilla Lyra Character Cosmetic System**:
   * Uses modular "Parts" driven via CopyPose from the main character (e.g., ShooterMannequin).
   * Recommended for those already familiar with Lyra's default system. 
     * It is recommended to review the Lyra documentation for more information on how to use this system.
2. **Extended Mutable System**:
    * Provides a streamlined approach to customization
    * Supports runtime mesh modification for greater optimization and flexibility. 
    * Highly efficient! Merged skeletal mesh at runtime!
    * Highly recommend reviewing Mutable documentation for deeper understanding.


## 📦 Installation


### From Lyra 

If coming from Lyra, it is recommended to initially inherit from 

```cpp
* ALyraTaggedActor : public ARockTaggedActor`
* class ULyraControllerComponent_CharacterParts : public URockControllerComponent_CharacterParts`
* class ULyraPawnComponent_CharacterParts : public URockPawnComponent_CharacterParts`
```

or if you are ready to dive into Mutable 

```cpp
ALyraTaggedActor : public ARockMutableTaggedActor
* class ULyraControllerComponent_CharacterParts : public URockControllerComponent_CharacterParts`
* class ULyraPawnComponent_CharacterParts : public URockMutablePawnComponent_CharacterParts`
```

And can consider removing most of the old Lyra Cosmetic folder files.

#### **Advanced Note**: 
You can outright swap to inheriting from the new RockCosmetic classes from Blueprint as well and skip the above steps.

### From Scratch

1. **Clone the repository** into your project's `Plugins` folder.
2. **Enable the plugin** in your project's `Edit -> Plugins` menu.
3. **Add the plugin's module dependency** to your project's `Build.cs` file.
4. **Rebuild your project** to ensure the plugin is correctly integrated.
```csharp
PublicDependencyModuleNames.AddRange(new string[] { "RockCosmetics" });
```
5. **Create a new `CustomizableObject`** in your project's `Content Browser`. See Mutable for more information
6. Create a new BP and inherit from RockMutableTaggedActor and set the CustomizableObject to the one you created in step 5.
7. Add RockMutablePawnComponent_CharacterParts to your pawn and set the CustomizableObject to the one you created in step 5.
8. Add RockControllerComponent_CharacterParts to your controller.
9. Set your main PlayerPawn to an equivalent to SKM_Manny_Invis.  (See Lyra's sample project for more information)
10. On beginPlay, call AddCharacterPart with new BP from step 6. AddCharacterPart is part of ControllerComponent_CharacterParts.  

WIP: More detailed instructions will be added in the future. 


## 📜 Why use this plugin?

* **Optimization**:
  * Unlike many cosmetic systems, Lyra's CharacterParts are not spawned on dedicated servers, saving valuable resources.
  * Allows for a 'higher fidelity' character for the player and a 'lower fidelity' character for others. Ideal for performance balancing
  * Combined with Mutable, a single skeletal mesh can be dynamically modified at runtime for further reducing memory and draw calls.

* **Why use this plugin over Lyra's original**:
  * More extensible and designed to support Mutable
  * encourages community contributions for continued improvements
  * Some existing modularization efforts for Lyra have unclear licensing concerns. This plugin clearly follows Epic's EULA first, then MIT License for any remaining portions.

# License

This project is a fork of Epic Games' codebase and is therefor subject to the Unreal Engine EULA and the Unreal Engine Content EULA.
Any portions of the code that are not covered by Epic's EULA are licensed under the MIT License, as specified in the LICENSE file.

# 🔄Notable changes from original
(Not an exhaustive list)

* New Actors and Components
    * RockSkeletalTaggedActor and RockMutableTaggedActor, extended from Lyra's TaggedActor. And RockMutablePawnComponent_CharacterParts
    * RockMutableTaggedActor supports Mutable network replication
    * Recommended to use RockMutableTaggedActor in conjunction with RockMutablePawnComponent_CharacterParts
  * Blueprint Function Libraries
    * Added BuildCustomizableActor and BuildCustomizableActorWithInstance for assist in creating Mutable Characters.
  * Data Structures
    * Added FRockMutableOption struct for easily adding to a ItemFragment or whatever other system. (WIP)


# 🛣️ Roadmap

* Further refinement and improved integration with Mutable System
* Improve editor UI for FRockMutableOption (dropdowns based upon CustomizableObject instead of manually typed values).
* Replace `FindPlayInEditorAuthorityWorld` with  `ForEachAuthorityWorld` (WIP, was a TODO from original Lyra)


# 🔍What is Mutable?

Mutable is a powerful customization system for creating highly optimized, dynamic skeletal meshes and static meshes.

It allows for rea-time modifications such as:
 * Changing colors, textures, materials
 * Adding, removing, or modifying mesh parts dynamically.
 * Generating a runtime merged skeletal mesh. Eliminating the need for CopyPose or multiple character meshes. 
     * Enables and streamlines new workflows for 3D modelers.   

Unlike traditional systems, Mutable generates a single optimized skeletal mesh at runtime, drastically improving performance and memory usage.

Epic Games acquired Mutable from Anticto. Games using Mutable:
* Lego Fortnite
* PUBG


# 🔗 Relevant Links
* Announcement: https://www.unrealengine.com/en-US/news/the-mutable-sample-project-is-now-available
* Original Documentation: https://github.com/anticto/Mutable-Documentation/wiki
* Epic Tutorial: https://dev.epicgames.com/community/learning/tutorials/yjw9/unreal-engine-mutable-tutorials



