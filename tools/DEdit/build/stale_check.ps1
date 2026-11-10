# stale_check.ps1 - delete objects whose source, or any header it includes (transitively), really changed
# since the object was built.
#
# "Really changed" means the file CONTENT changed: a file that was only re-saved or copied over with the same
# content (new timestamp, same bytes) does not trigger a rebuild.  resource.h is not tracked: it only holds
# #define numbers, and a file that uses a new ID is itself edited.  Use "build_dedit.bat rebuild" if you
# renumber existing IDs.
param([string]$Root,[string]$SrcList,[string]$Obj)
$ErrorActionPreference='SilentlyContinue'
$ignore=@('resource.h')

# ---- content-change cache: full path | mtime ticks | sha1 | changed ticks ----
$cacheFile=Join-Path (Split-Path $Obj -Parent) 'stale_cache.txt'
$cache=@{}
if(Test-Path -LiteralPath $cacheFile){
  foreach($l in Get-Content -LiteralPath $cacheFile){
    $p=$l.Split('|'); if($p.Count -eq 4){ $cache[$p[0]]=@{m=[int64]$p[1];h=$p[2];c=[int64]$p[3]} }
  }
}
$nowTicks=[datetime]::Now.Ticks
function ChangedTime($f){
  $k=$f.FullName.ToLower(); $m=$f.LastWriteTime.Ticks
  $e=$cache[$k]
  if($e -and $e.m -eq $m){ return $e.c }
  $h=(Get-FileHash -LiteralPath $f.FullName -Algorithm SHA1).Hash
  if($e -and $e.h -eq $h){ $cache[$k]=@{m=$m;h=$h;c=$e.c}; return $e.c }   # same bytes, new timestamp
  $c=$m; if($e){ $c=$nowTicks }                                            # content really changed
  $cache[$k]=@{m=$m;h=$h;c=$c}; return $c
}

$inc=@{}   # file name (lower) -> list of included header names
$paths=@{} # header name -> newest content-change time (ticks)
$files=Get-ChildItem -LiteralPath "$Root\tools","$Root\libs","$Root\sdk" -Recurse -Include *.h,*.hpp,*.inl
foreach($f in $files){
  $n=$f.Name.ToLower()
  if($ignore -contains $n){ continue }
  $t=ChangedTime $f
  if(-not $paths.ContainsKey($n) -or $t -gt $paths[$n]){ $paths[$n]=$t }
  $list=@(); foreach($m in Select-String -LiteralPath $f.FullName -Pattern '^\s*#\s*include\s*[<"]([^>"]+)[>"]'){ $list+=[IO.Path]::GetFileName($m.Matches[0].Groups[1].Value).ToLower() }
  if($inc.ContainsKey($n)){ $inc[$n]+=$list } else { $inc[$n]=$list }
}
$memo=@{}
function Newest($h,$seen){
  if($memo.ContainsKey($h)){ return $memo[$h] }
  if($seen.ContainsKey($h)){ return [int64]0 }
  $seen[$h]=1
  $t=[int64]0; if($paths.ContainsKey($h)){ $t=$paths[$h] }
  foreach($c in $inc[$h]){ $x=Newest $c $seen; if($x -gt $t){ $t=$x } }
  $memo[$h]=$t; return $t
}
$n=0
foreach($line in Get-Content -LiteralPath $SrcList){
  $src=$line.Trim(); if(-not $src -or $src.StartsWith('#')){ continue }
  $name=($src -replace '\\','_') -replace '\.cpp$','' -replace '\.c$',''
  $o=Join-Path $Obj ($name+'.obj'); $s=Join-Path $Root $src
  if(-not (Test-Path -LiteralPath $s)){ continue }
  $sc=ChangedTime (Get-Item -LiteralPath $s)
  if(-not (Test-Path -LiteralPath $o)){ continue }
  $ot=(Get-Item -LiteralPath $o).LastWriteTime.Ticks
  $stale=($sc -gt $ot)
  if(-not $stale){
    foreach($m in Select-String -LiteralPath $s -Pattern '^\s*#\s*include\s*[<"]([^>"]+)[>"]'){
      $h=[IO.Path]::GetFileName($m.Matches[0].Groups[1].Value).ToLower()
      if((Newest $h @{}) -gt $ot){ $stale=$true; break }
    }
  }
  if($stale){ Remove-Item -LiteralPath $o -Force; $n++ }
}
$out=foreach($k in $cache.Keys){ "{0}|{1}|{2}|{3}" -f $k,$cache[$k].m,$cache[$k].h,$cache[$k].c }
Set-Content -LiteralPath $cacheFile -Value $out
Write-Host ("Changed sources/headers: " + $n + " object(s) will be recompiled")
