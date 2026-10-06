# nur.h Development Environment Setup Script (PowerShell)
# Sets up complete development environment for nur.h library

param(
    [switch]$Force = $false
)

# Colors for output (Windows PowerShell compatible)
$script:Red = if ($Host.UI.SupportsVirtualTerminal) { "`e[31m" } else { "" }
$script:Green = if ($Host.UI.SupportsVirtualTerminal) { "`e[32m" } else { "" }
$script:Yellow = if ($Host.UI.SupportsVirtualTerminal) { "`e[33m" } else { "" }
$script:Blue = if ($Host.UI.SupportsVirtualTerminal) { "`e[34m" } else { "" }
$script:Reset = if ($Host.UI.SupportsVirtualTerminal) { "`e[0m" } else { "" }

function Write-ColorMessage {
    param($Message, $Color = "")
    Write-Host "$Color$Message$script:Reset"
}

Write-ColorMessage "🚀 Setting up nur.h development environment..." $script:Blue

# Check if we're in the right directory
if (-not (Test-Path "src\nur.h")) {
    Write-ColorMessage "❌ Error: src\nur.h not found. Run this script from the nur.h project root." $script:Red
    exit 1
}

Write-ColorMessage "📁 Project structure verified" $script:Blue

# Check for required tools
Write-ColorMessage "🔧 Checking development tools..." $script:Yellow

function Test-Tool {
    param($ToolName)
    try {
        $null = Get-Command $ToolName -ErrorAction Stop
        Write-ColorMessage "✅ $ToolName found" $script:Green
        return $true
    } catch {
        Write-ColorMessage "❌ $ToolName not found" $script:Red
        return $false
    }
}

# Essential tools check
$missingTools = 0

# Check for compilers
$hasGcc = Test-Tool "gcc"
$hasClang = Test-Tool "clang" 
$hasMsvc = Test-Tool "cl"

if (-not ($hasGcc -or $hasClang -or $hasMsvc)) {
    Write-ColorMessage "❌ No C compiler found. Install GCC, Clang, or Visual Studio." $script:Red
    $missingTools++
}

# Check for make or alternative
$hasMake = Test-Tool "make"
$hasNmake = Test-Tool "nmake"
$hasMsbuild = Test-Tool "msbuild"

if (-not ($hasMake -or $hasNmake -or $hasMsbuild)) {
    Write-ColorMessage "❌ No build system found. Install make, Visual Studio, or MinGW." $script:Red
    $missingTools++
}

# Optional tools
if (-not (Test-Tool "git")) {
    Write-ColorMessage "⚠️  git recommended for version control" $script:Yellow
}

if ($missingTools -gt 0 -and -not $Force) {
    Write-ColorMessage "❌ Missing $missingTools essential tools. Use -Force to continue anyway." $script:Red
    exit 1
}

# Try compilation test
Write-ColorMessage "🔨 Testing compilation..." $script:Yellow

$compileSuccess = $false

if ($hasGcc) {
    try {
        & gcc -std=c11 -Wall -Wextra -Wpedantic -fsyntax-only src\nur.h
        if ($LASTEXITCODE -eq 0) {
            Write-ColorMessage "✅ Header compiles successfully with GCC" $script:Green
            $compileSuccess = $true
        }
    } catch {
        Write-ColorMessage "⚠️  GCC compilation test failed" $script:Yellow
    }
}

if ($hasClang -and -not $compileSuccess) {
    try {
        & clang -std=c11 -Wall -Wextra -Wpedantic -fsyntax-only src\nur.h  
        if ($LASTEXITCODE -eq 0) {
            Write-ColorMessage "✅ Header compiles successfully with Clang" $script:Green
            $compileSuccess = $true
        }
    } catch {
        Write-ColorMessage "⚠️  Clang compilation test failed" $script:Yellow
    }
}

if ($hasMsvc -and -not $compileSuccess) {
    try {
        & cl /std:c11 /Zs src\nur.h 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-ColorMessage "✅ Header compiles successfully with MSVC" $script:Green
            $compileSuccess = $true
        }
    } catch {
        Write-ColorMessage "⚠️  MSVC compilation test failed" $script:Yellow
    }
}

if (-not $compileSuccess) {
    Write-ColorMessage "❌ Compilation failed with all available compilers" $script:Red
    if (-not $Force) { exit 1 }
}

# Try to build and run tests
Write-ColorMessage "🧪 Building and running tests..." $script:Yellow

$testSuccess = $false

if ($hasMake) {
    try {
        & make test
        if ($LASTEXITCODE -eq 0) {
            Write-ColorMessage "✅ All tests pass" $script:Green  
            $testSuccess = $true
        }
    } catch {
        Write-ColorMessage "⚠️  Make test failed" $script:Yellow
    }
}

# Manual test build if make failed
if (-not $testSuccess -and $hasGcc) {
    try {
        & gcc -std=c11 -Wall -Wextra -Isrc -o test_nur.exe test\test_nur.c
        if ($LASTEXITCODE -eq 0) {
            & .\test_nur.exe
            if ($LASTEXITCODE -eq 0) {
                Write-ColorMessage "✅ Manual test build successful" $script:Green
                $testSuccess = $true
            }
            Remove-Item "test_nur.exe" -ErrorAction SilentlyContinue
        }
    } catch {
        Write-ColorMessage "⚠️  Manual test build failed" $script:Yellow
    }
}

# Setup git hooks if git is available and .git exists
if ((Test-Path ".git") -and (Get-Command "git" -ErrorAction SilentlyContinue)) {
    Write-ColorMessage "🪝 Setting up git hooks..." $script:Yellow
    
    $hookPath = ".git\hooks\pre-commit"
    if (-not (Test-Path $hookPath) -or $Force) {
        @"
#!/bin/bash
# nur.h pre-commit hook  
echo "Running pre-commit checks..."
make check || exit 1
make test || exit 1
echo "Pre-commit checks passed!"
"@ | Out-File -FilePath $hookPath -Encoding ASCII
        
        Write-ColorMessage "✅ Git pre-commit hook installed" $script:Green
    }
}

# Create IDE configuration
Write-ColorMessage "⚙️  Creating IDE configuration..." $script:Yellow

# VS Code settings
$vscodeDir = ".vscode"
if (-not (Test-Path $vscodeDir)) {
    New-Item -ItemType Directory -Path $vscodeDir -Force | Out-Null
}

$cppProperties = @"
{
    "configurations": [
        {
            "name": "nur.h", 
            "includePath": ["`${workspaceFolder}/src"],
            "defines": [],
            "compilerPath": "gcc",
            "cStandard": "c11",
            "intelliSenseMode": "gcc-x64",
            "compilerArgs": [
                "-Wall",
                "-Wextra",
                "-Wpedantic"
            ]
        }
    ],
    "version": 4
}
"@

$cppProperties | Out-File -FilePath "$vscodeDir\c_cpp_properties.json" -Encoding UTF8

$tasks = @"
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build tests",
            "type": "shell",
            "command": "make",
            "args": ["test"],
            "group": {
                "kind": "build", 
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            }
        },
        {
            "label": "compile with gcc",
            "type": "shell",
            "command": "gcc",
            "args": ["-std=c11", "-Wall", "-Wextra", "-Isrc", "-o", "test_nur.exe", "test/test_nur.c"],
            "group": "build"
        }
    ]
}
"@

$tasks | Out-File -FilePath "$vscodeDir\tasks.json" -Encoding UTF8

Write-ColorMessage "✅ VS Code configuration created" $script:Green

# Summary
Write-ColorMessage "`n🎉 Development environment setup complete!" $script:Green
Write-ColorMessage "`n📋 Quick Start Commands:" $script:Blue
Write-ColorMessage "  make test        - Run test suite" $script:Yellow
Write-ColorMessage "  make check       - Check compilation" $script:Yellow  
Write-ColorMessage "  make clean       - Clean build artifacts" $script:Yellow
Write-ColorMessage "`n📁 Key Files:" $script:Blue
Write-ColorMessage "  src\nur.h       - Main library header" $script:Yellow
Write-ColorMessage "  test\test_nur.c  - Test suite" $script:Yellow
Write-ColorMessage "  README.md        - Documentation" $script:Yellow

Write-ColorMessage "`n🚀 Happy coding!" $script:Green