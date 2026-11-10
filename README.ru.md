# Condemned Map Kit — редактор и сборщик карт для Condemned: Criminal Origins

Нужно: Windows 10/11 и установленная игра Condemned: Criminal Origins (ПК-версия). Файлов игры в наборе нет —
`Setup` подключает вашу установку (папки ресурсов — ссылки, ничего не копируется и не меняется в игре).

## Быстрый старт (всё как в F.E.A.R.)
1. Распакуйте набор в папку без кириллицы и пробелов, например `C:\CondemnedMapKit`.
2. Один раз запустите **`Setup.cmd`** и укажите папку игры (где лежит `Condemned.exe`). Он подключает вашу установку и
   прописывает проект в настройках редактора.
3. Запускайте **`Tools\WorldEdit_Condemned_v4.exe`** обычным двойным щелчком (или `Start-Editor.cmd`): редактор сам
   откроет проект набора и шаблонную карту `123`. Карты лежат в `Runtime\Game\Worlds` — сохраняйте свои туда
   (`File > Save As`, `.World00c`).
4. Сборка — из редактора: **Ctrl+P** (World Packer), вкладки Common/Advanced как у оригинала, кнопка **Process**.
   Рядом с картой появятся `ИмяКарты.World00p`, `.WorldClient00p`, `.WorldServer00p`.
5. Запуск игры на карте — из редактора: **Ctrl+Alt+R** (Run). Игра стартует в окне сразу на этой карте.
   Прыгать в игре нельзя.

Запасные способы без редактора: `Build-Map.cmd` (перетащить `.World00c`/`.World00a`) и `Run-Map.cmd Worlds\ИмяКарты`.
## Что внутри
- `Tools\WorldEdit_Condemned_v4.exe` — WorldEdit из SDK F.E.A.R., доработанный под Condemned (классы/базы берутся из родной
  `GameServer.dll` игры).
- `Tools\packCDM\WorldPacker.dll` — **сборщик миров для Condemned** (восстановлен; формат v113: три файла вместо одного).
  Ресурсы Streaming собирает родной `GameServer.dll` игры, как это делал оригинал. Рядом лежат `vcruntime140.dll`/`msvcp140.dll`.
- `Tools\winpacker.exe`, `Tools\cdmpacker.exe` — оболочка сборщика.
- `Runtime\CondemnedTestLauncher.exe` — запуск игры на выбранной карте.
- `Maps\` — исходники примеров: `CdmRoom` (две комнаты), `CdmRoomNav` (+навигация), `CdmRoomAI` (+враг),
  `CdmRoomFull` (+оружие, реквизит), `CdmRoomDoor` (+дверь в проёме), `CdmRoomTrigger` (запертая дверь, которую открывает триггер), `CdmRoomRegions` (два региона стриминга; можно SplitMeshes TRUE); `make_room_map.py` — генератор таких карт (нужен Python 3).
- `source\` — исходники сборщика и заметки о формате (`FORMAT.md`, `GATHERER.md`).

## Проверено
Карты примеров собираются и загружаются в игре: геометрия, свет, коллизия стен, навигация (Nav), враг (ИИ), подбор оружия.
Не проверено: двери (RotatingDoor), триггеры и скрипты-команды, стриминг-регионы с ресурсами по регионам (ресурсы идут в общий
список), `GatherAssetListResources`, режимы `ObjectsOnly`/`CreateObjectFile`.

## Полезное (проверено в игре)
- Триггеры шлют команды вроде `msg CdmDoor00 unlock;msg CdmDoor00 on` (глаголы двери: `on` — открыть, `off` — закрыть,
  `lock`, `unlock`; команды `open` нет). Пример: `Maps\CdmRoomTrigger.World00a`.

## Если что-то не работает
- **Ошибка редактора «Can't find class module GameServer.dll»** — редактор не может загрузить `Runtime\Game\GameServer.dll`. Запустите `Check.cmd`: он покажет, чего именно нет, и исправит путь проекта в редакторе.
  Частые причины: не запускали `Setup.cmd` (или запускали из архива), папку набора переместили после Setup (один раз запустите `Start-Editor.cmd`), в пути русские буквы, в Setup указана не та папка игры.
- «Run Setup.cmd first» — не выполнен `Setup.cmd`.
- Сборка не создала файлы — запустите `Build-Map.cmd` из консоли и проверьте, что карта лежит в `Runtime\Game\Worlds`.
- Нет `vcruntime140.dll` и т.п. — они уже в `Tools\packCDM`; на Windows 7 нужен обновлённый UCRT.
- Не меняйте файлы в папке игры: набор их только читает.

Набор — исследовательский инструмент для моддинга на личном компьютере. Редактор и часть библиотек происходят из SDK F.E.A.R.
(Monolith Productions); не распространяйте их публично.
