# Changelog

## 2610.0901

- Tests module is `UncookedOnly` instead of `DeveloperTool`, so the staged game target no longer links it (T-220).

## 2610.0302
- Fixed `FRockCharacterPartList::SpawnActorForEntry` logging a warning for every part added on a dedicated server; it now returns quietly (a null owner still trips the `ensure`).

## 2610.0301
- Added the `RockCosmeticsTests` Developer module (`BRS.RockCosmetics.*`): part requests, body style and anim layer selection, Mutable layer list, pawn component add/remove/tags/replication callbacks, Mutable recompose bookkeeping.
