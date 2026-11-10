# Condemned: Criminal Origins WorldPacker (restored)

Общайся по-русски. Это ОТДЕЛЬНЫЙ проект. Не смешивать с FEAR-деревом
`..\worldpacker-rebuild` (его код только читать как образец; правки в нём не делать,
если владелец не попросил). Данные из Downloads — только чтение.

## Исходные условия (слова владельца, 31.10.2026)
- Оригинальный пакер Condemned никогда не публиковался. Его нет и в поставке игры.
- Доступные инструменты — FEAR-овские (`Tools\WorldPacker.dll`, `winpacker.exe`, `WorldEdit.exe`).
- Формат карт Condemned на ~95% совпадает с FEAR. Значит, база — наш FEAR-пакер,
  а различия выясняются по скомпилированным мирам Condemned.

## Пути (только чтение)
- Игра: `C:\Users\user\Downloads\condemned_20251104\Condemned - Criminal Origins` (Game=DARK, GameServer.dll 2024-06-02).
- Инструменты: `C:\Users\user\Downloads\condemned_20251104\Tools`.
- Корпус эталонов: 30 `.World00p` + 68 `.Mesh00p` в `Game\m01..m07_08` (`inventory\compiled-worlds.csv`).
  Исходников (`.World00a/c`) нет — только готовые выходы. Это эталон формата, не вход.

## Хэши (inventory\tools-hashes.json)
- `Tools\WorldPacker.dll` = FEAR-оригинал, SHA 4154e95e… (тот же, что исследовали в FEAR-проекте).
- `Tools\Packers\WorldPacker.dll` (913408 Б, 48a654c6…) — НЕ оригинал: timestamp 2026-10-21, это наша
  монолитная FEAR-сборка. Не использовать как эталон.

## Правила
Те же, что в FEAR-проекте: без донорских секций и таблиц под карты, без маскировки допусками,
непрошедшие отчёты не переписывать. Эталон Condemned = только то, что читает его GameServer/GameClient.dll.

## Состояние (31.10.2026)
- `fear-base\` — форк FEAR-кода для Condemned (правки помечены `CONDEMNED FORK`): профиль ресурсов `none`,
  `CondemnedContainer.h` (FEAR-вид → WLDP/WLDC/WLDS), `native-monolith-dll.cpp` пишет тройку.
- Сборка: `python -I fear-base\build-dll.py` → `dist\packers\WorldPacker.dll` (хост для проб: `build-host.py`).
  Nav SDK-объекты берутся read-only из FEAR-дерева (`worldpacker-rebuild\nav-sdk-native`).
- Запуск: `winpacker.exe -file X.World00c -ProjectPath <Runtime\Game> -PackerDir dist\packers -SkipPropDlg ...`
  (winpacker из Condemned-WorldEdit\Tools). Результат: X.World00p/.WorldClient00p/.WorldServer00p рядом с исходником.
- Игровая проверка: 123.World00c (CdmTest123, CdmDll123) загружается в Condemned через CondemnedTestLauncher.
- Тестовая карта: `maps\make_room_map.py` → `maps\CdmRoom.World00a` (2 комнаты, проём, 2 ящика, 2 света, старт).
  Пакуется winpacker'ом (World00p 8089 / Client 13599 / Server 1536 Б), в игре рендерится (стены/пол/свет/проём),
  игрок ходит. Владелец подтвердил: «карта работает». Чёрный силуэт в кадре — тень самого игрока, затем игрок
  упёрся в стену и она закрыла свет (не дефект пакера); игрок упирается в стены (коллизия стен работает).
  В игре нельзя прыгать — не проверять прыжками.
- `maps\make_room_map.py <out> nav` → CdmRoomNav: + AINavMesh (плоскость 960x960, 2 карвера на перегородках) и
  AINodeStalk с дочерней кистью. Форк читает `Database\Dark.Gamdb00p` (NavDatabaseManifest.h). Пакуется:
  World00p 30125 Б, BlindData = Stalk v1 (72 Б) + Nav v1 (21936 Б); в игре загружается, вид тот же.
- `maps\make_room_map.py <out> nav ai` → CdmRoomAI: + CAI (ModelTemplate M01_Addict3ONESHOT, Brain Default, GoalSet Patrol,
  схема свойств взята из ретейл-WorldServer, `research\parse_wlds.py`). Для этого в профиле `none` форк пропускает
  ресурсы классов (SourceMaterialSeeds.h). В игре ИИ появился, атаковал игрока (здоровье упало), владелец его
  убил — лежащая модель на кадре это труп ИИ. Владелец: «всё сработало». (Моя первая трактовка кадра как
  смерти игрока была неверной.)
- **Шлюз ресурсов (31.10.2026)**: профиль `condemned` (по умолчанию в DLL). `CondemnedGatherer.h` реализует
  `IObjectResourceGatherer` (раскладка Condemned = FEAR + 1 слот после деструктора), загружает `Game\GameServer.dll`
  проекта, вызывает `DEditLoadModule(Game)` (как WorldEdit; открывает базы), затем Pre → функции классов (порядок
  модели) → Post. На CdmRoomAI: 564 глобальных ресурса (звуки, текстуры, модели, оружие) от родного GameServer.
  Только глобальный регион (региональные ресурсы идут в global); `GatherAssetListResources` не подключён
  (сигнатура не выяснена, падает с заглушкой). Отладка: env `CONDEMNED_GATHER_TRACE=<файл>`; зонд
  `research\gather_probe.cpp`. CdmRoomFull (+ WeaponItem Pipe/2x4, Prop crow): в игре «Grab Small Pipe» работает.
- **Набор для друга**: `package-template\` + `build_package.ps1` → `dist\CondemnedMapKit` и `dist\CondemnedMapKit.zip`
  (18 МБ / 6 МБ; без файлов игры). Setup.cmd подключает установленную игру (junction'ы, копии GameServer.dll и т.п.),
  Start-Editor / Build-Map / Run-Map. Проверено из чистой копии: Setup → сборка CdmRoomFull → запуск игры; редактор
  открывает Template.World00c. Пересобирать набор после каждой пересборки DLL.
- **Штатный путь через редактор проверен (31.10.2026)** из чистой копии набора: WorldEdit запущен без аргументов →
  сам открыл проект и мир `123` → Ctrl+P открыл World Packer (cdmpacker.exe + packCDM\WorldPacker.dll) → Process →
  «Finished Packing», Errors 0 → Ctrl+Alt+R запустил игру на карте. Причина прежней ошибки «Error opening world: 123»:
  `FEAR.lyt` игры требует `Worlds\123.World00c` — шаблон теперь копируется под этим именем. `Setup.ps1` пишет в HKCU
  `CDMD\WorldEdit\Misc\LastProject/AutoLoadProj`, `Recent Project Files` (v4) и `Run\ProgramArguments`.
- **Дверь (31.10.2026)**: `make_room_map.py <out> nav props door` → CdmRoomDoor (RotatingDoor + дочерняя кисть DoorPanel,
  свойства из ретейла, звуки door_mtl_opn_3/cls_3 разрешаются через шлюз). Упаковка и загрузка в игре работают, у двери
  появляется подсказка «Close Door». Движение двери автоматическими нажатиями не подтверждено — нужна проверка владельца.
  Исправлен баг генератора: список свойств затенял флаг `props`. Архив с дверью — `dist\CondemnedMapKit_v2.zip`
  (прежний `CondemnedMapKit.zip` был занят другим процессом при пересборке).
- Дверь CdmRoomDoor подтверждена владельцем («всё работает»). Следующая проверка: CdmRoomTrigger
  (`make_room_map.py <out> nav props door trigger`): дверь заперта, Trigger (Dims 200,100,60, z=-150) шлёт `msg CdmDoor00 open`.
- **ПОДТВЕРЖДЕНО владельцем (31.10.2026, «получилось»)**: CdmRoomTrigger — Trigger с `msg CdmDoor00 unlock;msg CdmDoor00 on`
  открывает дверь сам. Причина прежней неудачи — глагол: у дверей в ретейле команды `on/off/lock/unlock`, команды `open` нет
  (по ретейл-командам: on 245, unlock 175, off 150, lock 131). Архив: `dist\CondemnedMapKit_v4.zip`.
  Ниже — история ошибочного промежуточного вывода:
- (история) Триггер/команды — НЕ подтверждены (поправка 31.10.2026): CdmRoomTrigger собирается и загружается (Trigger, Command1
  `msg CdmDoor00 unlock;msg CdmDoor00 open`, дверь Locked=1), но мой вывод «триггер открыл дверь» был ошибочным: пока шёл мой
  прогон, владелец сам играл в том же окне и открыл дверь клавишей E (кадры v0/v1 показывают игрока уже у открытой двери до моих
  нажатий). Неясно, срабатывает ли Trigger и действует ли Locked. Нужна чистая проверка без вмешательства.
  Архив `dist\CondemnedMapKit_v3.zip` содержит эту карту как непроверенный пример.
  Урок по тестам: перед отправкой клавиш проверять, что окно «Condemned» в фокусе (иначе нажатия уходят в терминал).
- **Большие карты (31.10.2026)**: форк поднял лимиты BSP (глубина 1500, фрагменты 1e6, точки 1e7 вместо 256/1e5/1e6 — было
  «Prepared BSP depth limit exceeded») и добавил 32-битные индексы physics-меша при >65535 вершин (SourcePhysicsVertexIndex,
  PhysicsMeshPrefix, NativeMoppPrefix/NativeMoppCompiler `Mesh.indices32`, PhysicsMeshBlock). Стресс-карта `maps\make_stress_map.py`
  (9000 брашей, 36 секторов, 72 тыс. вершин physics): сборка хостом 95 с / 644 МБ, winpacker 107 с, три файла валидны
  (probe_wldp/probe_wldc), в игре загружается, игрок стоит на полу. Одна карта в одном секторе упирается в лимит render-узла
  (>65536 теневых треугольников) — нужны несколько секторов. FEAR-карта Office (850 КБ) — 6 с, WTF_Entry — остановка «WorldModel
  without brushes» (нет FEAR-префабов; как у оригинала).
  **MOPP**: на 9 из 9 ретейл-карт наша MOPP при тех же вершинах/индексах отличается от ретейловой по длине (на 0,6–3,5%),
  CodeInfo (16 байт) совпадает. Формат тот же (коллизия работает), но построитель Condemned настроен иначе, побайтного совпадения
  нет (`research\mopp_wide_check.py`). Оригинального Condemned-пакера нет — подбирать параметры можно только по размерам/байтам ретейла.
- **Регионы стриминга (31.10.2026)**: `make_room_map.py <out> nav props door trigger regions` → CdmRoomRegions (2 StreamingRegion с дочерними кистями-объёмами, StreamingInfo, 2 сектора). Streaming: regions=2, у каждого 1 сектор и 3 региональных ресурса, global 252; SplitMeshes TRUE даёт `CdmRoomRegions_RegionA/B.Mesh00p` (MESH v1), WLDC видит 3 меша и проходит probe. Урок: сектора должны КАСАТЬСЯ плоскостью, чтобы появился портал (зазор 2 ед. → portals=0 → следующая комната чёрная); сектор должен лежать ровно в одном регионе. В игре комната B видна через проём. Архив `dist\CondemnedMapKit_v6.zip`.
- **ObjectsOnly/CreateObjectFile (31.10.2026)**: в раскладке Condemned объекты в WorldServer00p, BlindData в World00p, поэтому ObjectsOnly = полная пересборка (3 файла) с предупреждением в журнале, CreateObjectFile = пишется только WorldServer00p. Проверено по меткам времени через winpacker (-CreateObjectFile/-ObjectsOnly). Настройки winpacker (реестр) возвращены к умолчаниям. Архив `dist\CondemnedMapKit_v7.zip`.
- **Региональные ресурсы (31.10.2026)**: шлюз хранит цель каждого ресурса (глобально / именованный регион / AABB-OBB). `CondemnedGatherer::Resolve` сопоставляет их с регионами стриминга (по имени; для боксов — по пересечению с выпуклыми объёмами региона, иначе глобально), `SourceMaterialSeeds::AppendRegional` добавляет сиды с индексом региона. CdmRoomRegions: global 252→247, регион A 3→5, регион B 3→6, всего ссылок 258 как раньше; с SplitMeshes TRUE карта собирается и грузится. Архив `dist\CondemnedMapKit_v8.zip`.
- Ограничения (устарело, см. выше): ресурсы Streaming пока пустые (`none`), BlindData Condemned-специфика не сверена,
  ObjectsOnly/CreateObjectFile остаются в FEAR-виде, MOPP для >65535 вершин не проверен.

## План
1. Парсер заголовка и секций корпуса; сводка отличий версии/секций от FEAR (`research\`).
2. Дизассемблер GameServer.dll/GameClient.dll Condemned: читатели World00p → список отличий формата.
3. Из FEAR-кода перенести в `src\` копию с параметризацией «профиль Condemned»; восстановить различия
   (версия, StalkDataIndex v1 с центром, ShatterInfo, и т.д. — см. FEAR `NATIVE_STALK.md`).
4. Для проверки нужны исходные миры: взять собственные/FEAR-карты, собрать профилем Condemned,
   прочитать результат читателями Condemned (поведение движка), сверять с корпусом по статистике секций.
