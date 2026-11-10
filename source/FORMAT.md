# Формат Condemned v113 (по 10 кампейным картам) — 31.10.2026

Проверено `research\probe_wldp.py` / `probe_all_wldp.py` на M01P01…M08P01: WLDP разбирается до EOF на всех 10.

## Три файла вместо одного
| Файл | Содержимое (порядок) |
|---|---|
| `X.World00p` (WLDP) | `WLDP`,113, 36 Б векторов (min,max,offset) → tree(с 44) → WorldModels → ordinary blockers → physics → streaming → particle blockers → BlindData **до EOF** |
| `X.WorldClient00p` (WLDC) | `WLDC`,113, render (header 10 u32, inline+внешние сетки), **visibility в конце, до EOF** (M01P01: заголовок 8 u32 на смещении 3691819: 168 секторов, 186 порталов, 73 узла) |
| `X.WorldServer00p` (WLDS) | `WLDS`,113, objectCount, объекты FEAR-формата до EOF |
| `X_SRnn.Mesh00p` | MESH v1 (как FEAR SplitMeshes) |

FEAR: один файл, 56-байтный заголовок (113 + 4 смещения: render, visibility, objects, blind) + 36 Б векторов.

## Отличия от FEAR (подтверждены на 10/10 картах)
1. Контейнер: магик+версия, три файла (выше), нет таблицы смещений в WLDP.
2. Счётчики WorldModels (8 u32) **без XOR 399**.
3. Заголовок Streaming — **12 u32** (FEAR 11); 12-е слово = 0 на M01P01. Пул строк начинается с +48.
   Остальные 11 слов и агрегаты (assets, members, polys, vertices) сходятся с грамматикой FEAR.
4. Physics: у mesh-shape индексы треугольников **32-битные при >65535 вершин** (M02P01: 68622), иначе 16-бит.
   Следствие для MOPP: FEAR-билдер ограничен 16-бит subpart — для больших карт нужен 32-бит путь.
5. BlindData: тот же кадр `count, payload, payload, 12*count` (8+payload+12*count == остаток), лежит в WLDP.

## WLDC и контейнер (31.10.2026, продолжение)
- WLDC = грамматика FEAR render **без изменений** (header 10 u32, inline+внешние сетки, деревья, SkyPortal) + visibility до EOF
  (`research\probe_wldc.py`: 10/10 карт, агрегаты visibility сошлись, конец ровно на EOF).
- WLDS = FEAR Objects (count + записи) до EOF, без доп. полей.
- `src\condemned_container.py`: `condemned_to_fear` / `fear_to_condemned`. Round-trip тройка→FEAR-вид→тройка
  **побайтно равен на 10/10 карт** (`research\roundtrip.json`). Т.е. отличия контейнера исчерпываются:
  разбиение на 3 файла, XOR 399, 12-е слово streaming, 32-бит индексы mesh. Остальное содержимое — формат FEAR.

## Первый игровой результат (31.10.2026)
`fear-base\` — форк FEAR-кода (src + build-host.py; Nav SDK-объекты берутся read-only из FEAR-дерева).
Правка форка: профиль ресурсов `--post-profile none` (без FEAR sound/ClientFX баз). Цепочка:
`native-monolith-host` (--post-profile none) → FEAR-вид → `condemned_container.fear_to_condemned` → тройка.
`123.World00c` (Condemned WorldEdit) → CdmTest123.World00p/WorldClient00p/WorldServer00p (1186/2722/1729 Б) в
`Condemned-WorldEdit\Runtime\Game\Worlds`. Через CondemnedTestLauncher игра Condemned загрузила карту и работала
(35 с, отвечает, 134 МБ; на скриншоте HUD игрока и sky-текстуры комнаты). Это загрузка, не прохождение.

## Блокер для сборки новых карт (найден при пробе 123.World00c) — СНЯТ профилем none; полный профиль ресурсов остаётся задачей
`native-monolith-host` (FEAR) на Condemned-проекте падает в streaming: `GamePostResources::Build` (exit 38) — это
зашитый в наш код FEAR-профиль ресурсов GameServer. У Condemned свой GameServer.dll (2024-06 сборка 1.0.314.0),
значит ресурсы/классы/свойства нужно брать оттуда (у оригинала это IGameWorldPacker внутри GameServer.dll).

## BlindData Condemned (10 карт, 2730 записей)
Типы те же, что у FEAR: KeyFramer 0x6aaf0884 (2230, версия 1), Nav 0x83f47c31 (1 на карту, версия 1),
Stalk 0x6aaf0885 (202), Shatter 0x0a85 (251), Scatter 0x73f53a84 (41, только M08). Запись таблицы: (size, type, offset).
**Stalk версии 1**: version, center(3 float), height, count, точки. Центр = сумма точек во float по порядку / count,
побитно на 202/202 записей. Внесено в форк (`NativeStalk.h`).
Ресурсы Streaming: см. GATHERER.md.

## Не выяснено
- 12-е слово заголовка Streaming (всегда 0?), сверить на 10 картах.
- Состав render-заголовка WLDC и глобальных деревьев; порядок visibility внутри WLDC (точная граница).
- Содержимое и версии BlindData-записей Condemned (Stalk v1 с центром, Shatter, Nav).
- Записи Objects: классы/свойства по Condemned GameServer.dll.
- Старые одиночные World00p v110–112 (20 файлов) — не цель.
