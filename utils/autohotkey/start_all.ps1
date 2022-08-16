$files = Get-ChildItem -filter *.ahk -Path .  -Force -ErrorAction SilentlyContinue | Select-Object FullName
$files

Foreach($file in $files){
  &$file.FullName
}
