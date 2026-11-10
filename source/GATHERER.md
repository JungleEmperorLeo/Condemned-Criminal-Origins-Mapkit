# Ресурсы Streaming Condemned: что известно (31.10.2026)

Сейчас DLL пишет Streaming без игровых ресурсов (`--post-profile none`). Настоящий путь оригинала:
пакер реализует `IObjectResourceGatherer` (FEAR SDK: `engine\sdk\inc\iobjectresourcegatherer.h`), а
`GameServer.dll` вызывает его из экспортов `GatherResourcesPreObjects`, `GatherResourcesPostObjects`,
`GatherAssetListResources` (в FEAR-коде GatherAssetListResources нет в нашей реализации) и из
обработчиков классов (`GatherObjectResourcesFn`, регистрируются в `ObjectDLLSetup`).

## Раскладка vtable Condemned (по дизассемблеру GameServer.dll, `research\GameServer.asm`)
Слоты Condemned = слоты FEAR + 1 для всех методов после деструктора (слот 0). Проверено по вызовам в
`GatherResourcesPostObjects` (0x101436B0):
`[+0x08]` GetWorldName(слот2) · `[+0x18]` GetNumRegionSurfaceTypes(6) · `[+0x1C]` GetRegionSurfaceType(7) ·
`[+0x44]` AddResourceToRegion(17, 28 вызовов) · `[+0x74]` GetPropString(29, 5 аргументов) ·
`[+0x80]` GetObjectsOfClass(32, 4 аргумента). Слот 1 (`[+0x04]`) в этих функциях не вызывается;
владельцев переходник (`packCDM`) тоже его не использует (на двух картах). Его назначение не найдено.

## План родного профиля
1. Реализовать класс-шлюз с 34 слотами (слот 1 — заглушка, возвращающая 0/NULL и пишущая предупреждение).
2. Загружать `Game\GameServer.dll` проекта через LoadLibrary, вызвать `ObjectDLLSetup` для получения
   классов/функций ресурсов, затем Pre → объекты → Post → AssetList.
3. Результат `AddResourceToRegion` складывать в списки global/regional Streaming (см. `StreamingResources`).
4. Сверка: нет эталона (оригинала Condemned-пакера нет); проверять только разбором ретейл-карт —
   список ресурсов M01P01 в Streaming (5581 ссылок) против того, что соберёт шлюз на их Objects.
   Для этого понадобится читатель Objects/WorldServer и воссоздание сцены из WLDS.
