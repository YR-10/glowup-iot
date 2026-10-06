param([ValidateSet('build','upload','monitor','devices','init','compiledb')][string]$Action = 'build')
$projectRoot = Split-Path -Parent $PSScriptRoot
$env:PYTHONPATH = Join-Path $projectRoot '.tools\pylibs'
$env:PLATFORMIO_CORE_DIR = Join-Path $projectRoot '.tools\platformio'
$pythonRuntime = Join-Path $env:USERPROFILE '.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
if (-not (Test-Path -LiteralPath $pythonRuntime)) { throw 'Runtime lokal tidak tersedia. Gunakan PlatformIO IDE atau pio dari terminal.' }
Push-Location $projectRoot
try {
    switch ($Action) {
        'build' { & $pythonRuntime -m platformio run }
        'upload' { & $pythonRuntime -m platformio run -t upload }
        'monitor' { & $pythonRuntime -m platformio device monitor -b 115200 }
        'devices' { & $pythonRuntime -m platformio device list }
        'init' { & $pythonRuntime -m platformio init --ide vscode }
        'compiledb' { & $pythonRuntime -m platformio run -t compiledb }
    }
    if ($LASTEXITCODE -ne 0) { throw "PlatformIO keluar dengan kode $LASTEXITCODE" }
} finally { Pop-Location }
