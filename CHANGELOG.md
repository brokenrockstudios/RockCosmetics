# Changelog

## 2610.2302

- `URockMutablePawnComponent_CharacterParts`: binding to a part that is waiting for its CustomizableObject now happens in one helper used by both `BroadcastChanged` and `ExecuteRecompose`, so a recompose for a waiting part `BroadcastChanged` never saw is no longer dropped (T-440).

## 2610.2301

- `URockAnimSubsystem`: new native `RequestMontage` (answers exactly once, null on a miss, waits while the sets load) and `RegisterAnimSet`. `URockAnimSetDataAsset::PreloadAll` now returns its handle; keep it or the montages can be collected. Nothing preloads automatically yet (decision T-1119) (T-424).
- `URockAsyncAction_GetMontage` has an `OnFailed` pin; exactly one of `OnLoaded`/`OnFailed` fires. A set's async montage load now always calls back, also with null (T-424).
- `GetSkeletonClass` warns once per pawn instead of on every call, and picks the lexically first of several `Cosmetic.SkeletonClass` tags (warning once). Tests: `BRS.RockCosmetics.AnimSubsystem` (T-424).

## 2610.1301

- Fixed `ARockMutableTaggedActor` spawning without a mesh in the editor when its CustomizableObject wasn't compiled yet (the first PIE of a session, or compiled data invalidated by a changed dependency). It now requests the compile and builds its usages once it finishes; `URockMutablePawnComponent_CharacterParts` picks the part up then.
- Added `On Customizable Object Instance Ready` (Blueprint) / `OnCustomizableObjectInstanceReady` (native) to `ARockMutableTaggedActor`. Set initial instance parameters there instead of in BeginPlay, where the instance may not exist yet.

## 2610.0901

- Tests module is `UncookedOnly` instead of `DeveloperTool`, so the staged game target no longer links it (T-220).

## 2610.0302
- Fixed `FRockCharacterPartList::SpawnActorForEntry` logging a warning for every part added on a dedicated server; it now returns quietly (a null owner still trips the `ensure`).

## 2610.0301
- Added the `RockCosmeticsTests` Developer module (`BRS.RockCosmetics.*`): part requests, body style and anim layer selection, Mutable layer list, pawn component add/remove/tags/replication callbacks, Mutable recompose bookkeeping.
