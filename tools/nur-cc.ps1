# =============================================================================
# nur-cc.ps1 - Universal Developer CLI & Compile Guard for Windows (PowerShell)
# =============================================================================

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Exe = Join-Path $ScriptDir "nur-cc.exe"

if (-not (Test-Path $Exe)) {
    $Src = Join-Path $ScriptDir "nur-cc.c"
    if (Get-Command gcc -ErrorAction SilentlyContinue) {
        gcc -Wall -Wextra -std=c99 $Src -o $Exe
    } elseif (Get-Command clang -ErrorAction SilentlyContinue) {
        clang -Wall -Wextra -std=c99 $Src -o $Exe
    }
}

if (Test-Path $Exe) {
    & $Exe @args
    exit $LASTEXITCODE
} else {
    Write-Host "Error: Failed to find or compile nur-cc.exe" -ForegroundColor Red
    exit 1
}
