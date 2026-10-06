# Build script for OpenGlt project
Write-Host "Building OpenGlt project with CMake..." -ForegroundColor Green

# Check if CMake is available
try {
    $cmakeVersion = cmake --version 2>$null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake not found! Please install CMake and add it to your PATH." -ForegroundColor Red
        exit 1
    }
    Write-Host "Found CMake: $($cmakeVersion.Split("`n")[0])" -ForegroundColor Cyan
} catch {
    Write-Host "CMake not found! Please install CMake and add it to your PATH." -ForegroundColor Red
    exit 1
}

# Create build directory if it doesn't exist
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
    Write-Host "Created build directory" -ForegroundColor Yellow
}
Set-Location "build"

# Try different Visual Studio generators
$generators = @(
    "Visual Studio 18 2026",
    "Visual Studio 17 2022",
    "Visual Studio 16 2019", 
    "Visual Studio 15 2017"
)

$generatorFound = $false
foreach ($generator in $generators) {
    Write-Host "Trying generator: $generator" -ForegroundColor Yellow
    cmake .. -G $generator -A x64 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Successfully configured with $generator" -ForegroundColor Green
        $generatorFound = $true
        break
    }
}

if (-not $generatorFound) {
    Write-Host "Failed to configure with any Visual Studio generator!" -ForegroundColor Red
    Write-Host "Trying with default generator..." -ForegroundColor Yellow
    cmake ..
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake configuration failed!" -ForegroundColor Red
        exit 1
    }
}

# Build the project
Write-Host "Building project..." -ForegroundColor Yellow
cmake --build . --config Debug

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Executable location: build\bin\Debug\OpenGlt.exe" -ForegroundColor Cyan

# Check if executable exists
if (Test-Path "bin\Debug\OpenGlt.exe") {
    Write-Host "Executable found at: $(Resolve-Path 'bin\Debug\OpenGlt.exe')" -ForegroundColor Green
} else {
    Write-Host "Warning: Executable not found at expected location" -ForegroundColor Yellow
}

# Return to original directory
Set-Location ".."
