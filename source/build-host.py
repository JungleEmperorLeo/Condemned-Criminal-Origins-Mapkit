"""Build fear-base/packers/native-monolith-host.exe from the Condemned fork of src/.
Same flags as the FEAR build; the Nav SDK adapter and prebuilt retail-2006 SDK objects are
shared read-only from the FEAR tree (FEAR_ROOT) -- not copied, not modified."""
from pathlib import Path
import json, subprocess, sys
sys.stdout.reconfigure(encoding='utf-8')
R = Path(__file__).resolve().parent
FEAR_ROOT = Path(r'C:\Users\user\Documents\Codex\2026-10-09\ckeif\outputs\worldpacker-rebuild')
vc = sorted(Path('E:/MVS/VC/Tools/MSVC').iterdir())[-1]; bin = vc / 'bin/Hostx64/x86'
nav = json.loads((FEAR_ROOT / 'nav-sdk-native/build-config.json').read_text(encoding='utf-8'))
(R / 'packers').mkdir(exist_ok=True)
commands = [[str(bin / 'lib.exe'), '/nologo', '/def:src/native-monolith-kernel32.def', '/machine:X86', '/out:packers/native-monolith-kernel32.lib']]
for source, target, warnings in [('src/native-monolith-host.cpp', 'native-monolith-host', 3), ('src/legacy-nvtristrip/NvTriStrip.cpp', 'native-NvTriStrip', 3), ('src/legacy-nvtristrip/NvTriStripObjects.cpp', 'native-NvTriStripObjects', 3), ('src/legacy-nvtristrip/VertexCache.cpp', 'native-VertexCache', 3)]:
    flags = ['/Zc:forScope-', '/DNDEBUG'] if 'legacy-nvtristrip' in source else ['/bigobj']
    commands.append([str(bin / 'cl.exe'), '/nologo', '/c', '/O2', '/Oi-', '/W' + str(warnings), '/GS-', '/GR-', '/Zl', '/Zc:threadSafeInit-', '/I' + str(vc / 'include'), *flags, source, '/Fo:packers/' + target + '.obj'])
commands.append([nav['compiler'], '/c', *nav['compileFlags'], '/D' + nav['retail2006Define'], *['/I' + p for p in nav['includeDirs']], str(FEAR_ROOT / 'nav-sdk-native/adapter/native-nav-library.cpp'), '/Fo' + str(R / 'packers/native-nav-library.obj')])
commands.append([str(bin / 'link.exe'), '/nologo', '/SUBSYSTEM:CONSOLE', '/ENTRY:mainCRTStartup', '/MACHINE:X86', '/LARGEADDRESSAWARE', '/OUT:packers/native-monolith-host.exe',
                 *[f'packers/{n}.obj' for n in ['native-monolith-host', 'native-NvTriStrip', 'native-NvTriStripObjects', 'native-VertexCache', 'native-nav-library']], *nav['retail2006Objects'],
                 'packers/native-monolith-kernel32.lib', *nav['explicitLibraries'], *['/LIBPATH:' + p for p in nav['libDirs']], *nav['defaultLibraries']])
for command in commands:
    run = subprocess.run(command, cwd=R, capture_output=True, timeout=1800); text = (run.stdout + run.stderr).decode('cp866', errors='replace')
    if run.returncode: print(text[-6000:]); raise SystemExit(run.returncode)
print('built packers/native-monolith-host.exe')
