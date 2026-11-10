# Condemned Map Kit - installation check. Prints what is wrong and repairs what can be repaired safely
# (editor registry pointing to this folder). Run via Check.cmd; Start-Editor.cmd runs it silently first.
param([switch]$Quiet)
$kit = Split-Path -Parent $MyInvocation.MyCommand.Path
$problems = @()
function Say($m, $c = 'Gray') { if (-not $Quiet) { Write-Host $m -ForegroundColor $c } }
function Need($ok, $text) { if ($ok) { Say "  OK   $text" 'Green' } else { Say "  FAIL $text" 'Red'; $script:problems += $text } }

Say "Kit folder: $kit"
Need ($kit -match '^[\x20-\x7E]+$') 'folder path is plain ASCII (the editor is an old ANSI program; Cyrillic/accented letters in the path break module loading)'
Need ($kit -notmatch ' ') 'folder path has no spaces (safest)'
$rg = "$kit\Runtime\Game"
Need (Test-Path "$kit\game-install.txt") 'Setup was run (game-install.txt exists)'
Need (Test-Path "$rg\GameServer.dll") 'Runtime\Game\GameServer.dll exists  <-- the file the editor error is about'
if (Test-Path "$rg\GameServer.dll") { Need ((Get-Item "$rg\GameServer.dll").Length -gt 1MB) 'GameServer.dll looks like the real game module (larger than 1 MB)' }
Need (Test-Path "$rg\FEAR.dep") 'Runtime\Game\FEAR.dep exists'
Need (Test-Path "$kit\Runtime\FEAR.proj00") 'Runtime\FEAR.proj00 exists'
Need (Test-Path "$kit\Runtime\GameDatabase.dll") 'Runtime\GameDatabase.dll exists (copied from the game folder)'
foreach ($d in 'msvcr71.dll', 'msvcp71.dll', 'MFC71.dll') { Need ((Test-Path "$kit\Tools\$d") -or (Test-Path "$kit\Runtime\$d")) "$d is present (needed by the game module)" }
Need (Test-Path "$kit\Tools\packCDM\WorldPacker.dll") 'Tools\packCDM\WorldPacker.dll exists'
$linked = @(Get-ChildItem "$rg" -Directory -ErrorAction SilentlyContinue | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }).Count
Need ($linked -ge 10) "game resource folders are linked ($linked links; expected ~18)"
if (Test-Path "$kit\game-install.txt") {
    $game = (Get-Content "$kit\game-install.txt" -TotalCount 1).Trim()
    Need (Test-Path "$game\Condemned.exe") "game folder from Setup still exists: $game"
}
# editor registry must point at THIS folder (moving the kit after Setup breaks it)
$misc = 'HKCU:\Software\Monolith Productions\CDMD\WorldEdit\Misc'
$want = "$rg\FEAR.dep"
$have = (Get-ItemProperty $misc -ErrorAction SilentlyContinue).LastProject
if ($have -ne $want -and (Test-Path $want)) {
    New-Item -Path $misc -Force | Out-Null
    Set-ItemProperty -Path $misc -Name LastProject -Value $want
    Set-ItemProperty -Path $misc -Name AutoLoadProj -Value 'True'
    $recent = 'HKCU:\Software\Monolith Productions\CDMD\WorldEdit_Condemned_v4\Recent Project Files'
    New-Item -Path $recent -Force | Out-Null
    Set-ItemProperty -Path $recent -Name File1 -Value $want
    Say "  FIXED editor project path in the registry (was: $have)" 'Yellow'
} else { Need ($have -eq $want) 'editor remembers this folder as its project' }

if ($problems.Count) {
    Say '' ; Say 'What to do:' 'Yellow'
    Say '  1. Unzip the kit completely (do not run from inside the zip) into a plain folder such as C:\CondemnedMapKit.'
    Say '  2. Run Setup.cmd and give it the folder that contains Condemned.exe.'
    Say '  3. Start the editor with Start-Editor.cmd. Then run Check.cmd again.'
    exit 1
} else { Say '' ; Say 'Everything looks fine.' 'Green'; exit 0 }
