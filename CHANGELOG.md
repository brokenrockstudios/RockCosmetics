# Changelog

## 2610.1301

- Fixed `ARockMutableTaggedActor` spawning without a mesh in the editor when its CustomizableObject wasn't compiled yet (the first PIE of a session, or compiled data invalidated by a changed dependency). It now requests the compile and builds its usages once it finishes; `URockMutablePawnComponent_CharacterParts` picks the part up then.
- Added `On Customizable Object Instance Ready` (Blueprint) / `OnCustomizableObjectInstanceReady` (native) to `ARockMutableTaggedActor`. Set initial instance parameters there instead of in BeginPlay, where the instance may not exist yet.

## 2610.0901

- Tests module is `UncookedOnly` instead of `DeveloperTool`, so the staged game target no longer links it (T-220).

## 2610.0302
- Fixed `FRockCharacterPartList::SpawnActorForEntry` logging a warning for every part added on a dedicated server; it now returns quietly (a null owner still trips the `ensure`).

## 2610.0301
- Added the `RockCosmeticsTests` Developer module (`BRS.RockCosmetics.*`): part requests, body style and anim layer selection, Mutable layer list, pawn component add/remove/tags/replication callbacks, Mutable recompose bookkeeping.
