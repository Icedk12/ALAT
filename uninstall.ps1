$ErrorActionPreference = 'Stop'

$installDirectory = Join-Path $env:LOCALAPPDATA 'Programs\ALAT'
$userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
$pathEntries = @($userPath -split ';' | Where-Object {
    -not [string]::IsNullOrWhiteSpace($_) -and
    $_.Trim().Trim('"').TrimEnd('\') -ine $installDirectory.TrimEnd('\')
})
[Environment]::SetEnvironmentVariable('Path', ($pathEntries -join ';'), 'User')

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

if (Test-Path -LiteralPath $installDirectory) {
    Remove-Item -LiteralPath $installDirectory -Recurse -Force
}

if (($env:Path -split ';' | Where-Object { $_.TrimEnd('\') -ieq $installDirectory.TrimEnd('\') }).Count -gt 0) {
    $env:Path = ($env:Path -split ';' | Where-Object { $_.TrimEnd('\') -ine $installDirectory.TrimEnd('\') }) -join ';'
}

Write-Host 'Removed ALAT from the user PATH and installation directory.'
Write-Host "User settings and restore backup were kept under $env:APPDATA\ALAT"