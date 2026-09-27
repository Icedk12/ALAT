$ErrorActionPreference = 'Stop'

$projectRoot = $PSScriptRoot
$builtExecutable = Join-Path $projectRoot 'src\main.exe'
if (-not (Test-Path -LiteralPath $builtExecutable -PathType Leaf)) {
    throw "Build ALAT first. Expected executable: $builtExecutable"
}

$installDirectory = Join-Path $env:LOCALAPPDATA 'Programs\ALAT'
New-Item -ItemType Directory -Path $installDirectory -Force | Out-Null
Copy-Item -LiteralPath $builtExecutable -Destination (Join-Path $installDirectory 'alat.exe') -Force

foreach ($folderName in @('languages', 'accents')) {
    $sourceDirectory = Join-Path $projectRoot $folderName
    $targetDirectory = Join-Path $installDirectory $folderName
    New-Item -ItemType Directory -Path $targetDirectory -Force | Out-Null
    Get-ChildItem -LiteralPath $sourceDirectory -File | ForEach-Object {
        $targetFile = Join-Path $targetDirectory $_.Name
        if (-not (Test-Path -LiteralPath $targetFile)) {
            Copy-Item -LiteralPath $_.FullName -Destination $targetFile
        }
    }
}

$userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
$pathEntries = @($userPath -split ';' | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
$alreadyInstalled = $false
foreach ($entry in $pathEntries) {
    $normalizedEntry = $entry.Trim().Trim('"').TrimEnd('\')
    if ($normalizedEntry -ieq $installDirectory.TrimEnd('\')) {
        $alreadyInstalled = $true
        break
    }
}

if (-not $alreadyInstalled) {
    $pathEntries += $installDirectory
    [Environment]::SetEnvironmentVariable('Path', ($pathEntries -join ';'), 'User')
}

if (-not ('AlatPathBroadcast' -as [type])) {
    Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class AlatPathBroadcast {
    [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
    public static extern IntPtr SendMessageTimeout(IntPtr hWnd, uint message, IntPtr wParam, string lParam, uint flags, uint timeout, out IntPtr result);
}
"@
}
$broadcastResult = [IntPtr]::Zero
[AlatPathBroadcast]::SendMessageTimeout([IntPtr]0xffff, 0x001A, [IntPtr]::Zero, 'Environment', 0x0002, 5000, [ref]$broadcastResult) | Out-Null

if (($env:Path -split ';' | Where-Object { $_.TrimEnd('\') -ieq $installDirectory.TrimEnd('\') }).Count -eq 0) {
    $env:Path = "$installDirectory;$env:Path"
}

Write-Host "Installed ALAT to $installDirectory"
Write-Host 'The user PATH now includes ALAT. New command prompts can run: alat -h'