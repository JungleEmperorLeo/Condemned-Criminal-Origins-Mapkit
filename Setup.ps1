# Condemned Map Kit - first-time setup. Needs only an installed copy of the game.
# Usage:  Setup.cmd            (asks for the game folder)
#         Setup.cmd "D:\Games\Condemned - Criminal Origins"
param([string]$GameDir)
$ErrorActionPreference = 'Stop'
$kit = Split-Path -Parent $MyInvocation.MyCommand.Path

function Find-Game([string]$p) {
    if (-not $p) { return $null }
    $p = $p.Trim('"').TrimEnd('\')
    if (-not $p) { return $null }
    if ((Test-Path "$p\Condemned.exe") -and (Test-Path "$p\Game\GameServer.dll")) { return $p }
    if ((Test-Path "$p\Condemned - Criminal Origins\Condemned.exe")) { return "$p\Condemned - Criminal Origins" }
    # the inner "Game" folder (or any subfolder of the game) was given: use its parent that holds Condemned.exe
    $parent = Split-Path -Parent $p
    if ($parent -and (Test-Path "$parent\Condemned.exe") -and (Test-Path "$parent\Game\GameServer.dll")) { return $parent }
    return $null
}
$game = Find-Game $GameDir
if (-not $game) {
    foreach ($c in @('C:\Program Files (x86)\Condemned - Criminal Origins', 'C:\Program Files (x86)\Steam\steamapps\common\Condemned', 'C:\Games\Condemned - Criminal Origins')) {
        $game = Find-Game $c; if ($game) { break }
    }
}
while (-not $game) {
    $game = Find-Game (Read-Host 'Path to the Condemned folder (the one with Condemned.exe)')
    if (-not $game) { Write-Host 'Condemned.exe and Game\GameServer.dll were not found there.' -ForegroundColor Yellow }
}
Write-Host "Game folder: $game"
if ((Get-Item "$game\Game\GameServer.dll").Length -lt 1MB) { throw 'Unexpected GameServer.dll (this kit is built for the retail PC release).' }

# 1. remember the game location (read by CondemnedTestLauncher.exe)
Set-Content "$kit\game-install.txt" $game -Encoding ascii

# 2. Runtime\Game: the editor/packer project root. Resource folders are junctions to the game (nothing is copied or modified).
$rt = "$kit\Runtime"; $rg = "$rt\Game"
New-Item -ItemType Directory -Force $rg, "$rg\Worlds" | Out-Null
foreach ($d in Get-ChildItem "$game\Game" -Directory) {
    if ($d.Name -ieq 'Worlds') { continue }
    $link = "$rg\$($d.Name)"
    if (-not (Test-Path $link)) { cmd /c mklink /J "`"$link`"" "`"$($d.FullName)`"" | Out-Null }
}
foreach ($f in 'GameServer.dll', 'GameClient.dll', 'ClientFx.fxd', 'Interface.asset00', 'FEAR.dep', 'FEAR.lyt') {
    if (Test-Path "$game\Game\$f") { Copy-Item "$game\Game\$f" "$rg\$f" -Force }
}
# libraries the editor project loads next to the project
foreach ($f in 'GameDatabase.dll', 'StringEditRuntime.dll', 'MFC71.dll', 'mfc71u.dll', 'msvcp71.dll', 'msvcr71.dll', 'LTMemory.dll') {
    if (Test-Path "$game\$f") { Copy-Item "$game\$f" "$rt\$f" -Force }
}
Set-Content "$rg\WorldEdit.ini" "[Options]`r`nProgramArguments=--dev `"%WorldName%`"" -Encoding ascii

# 3. project file with absolute paths for this PC
$proj = @"
[General]
Game=FEAR
[Project Binaries]
ClientFX=Game\ClientFX.fxd
GameDatabase=GameDatabase.dll
GameServer=Game\GameServer.dll
[Project Directories]
ResourceRoot=Game\
ToolsRoot=..\Tools\
PC=Game\
[Tools Launch]
PCBinary=$kit\Runtime\CondemnedTestLauncher.exe
PCWorking=$kit\Runtime\
[Source Control]
Provider={2A258D42-4F88-4ec6-BB94-A264C0F9EE9C}
"@
Set-Content "$rt\FEAR.proj00" $proj -Encoding ascii

# 4. sample maps (sources only; compiled worlds are produced by Build-Map.cmd)
foreach ($f in Get-ChildItem "$kit\Maps" -File -ErrorAction SilentlyContinue) {
    $name = if ($f.Name -eq 'Template.World00c') { '123.World00c' } else { $f.Name }
    if ($f.Extension -in '.World00a', '.World00c') { Copy-Item $f.FullName "$rg\Worlds\$name" -Force }
}

# 5. WorldEdit remembers its last project in HKCU; point it at this kit so the editor opens it by itself
$depPath = "$rg\FEAR.dep"
$misc = 'HKCU:\Software\Monolith Productions\CDMD\WorldEdit\Misc'
New-Item -Path $misc -Force | Out-Null
Set-ItemProperty -Path $misc -Name LastProject -Value $depPath
Set-ItemProperty -Path $misc -Name AutoLoadProj -Value 'True'
Set-ItemProperty -Path $misc -Name LoadLYTFile -Value 'True'
$recent = 'HKCU:\Software\Monolith Productions\CDMD\WorldEdit_Condemned_v4\Recent Project Files'
New-Item -Path $recent -Force | Out-Null
Set-ItemProperty -Path $recent -Name File1 -Value $depPath
$run = 'HKCU:\Software\Monolith Productions\CDMD\WorldEdit\Run'
New-Item -Path $run -Force | Out-Null
Set-ItemProperty -Path $run -Name ProgramArguments -Value '--dev "%WorldName%"'

Write-Host ''
Write-Host 'Setup finished.' -ForegroundColor Green
Write-Host '  Start-Editor.cmd   - open WorldEdit with the template map'
Write-Host '  Build-Map.cmd      - compile a .World00c/.World00a (drag it onto the file)'
Write-Host '  Run-Map.cmd        - start the game on a compiled map'
