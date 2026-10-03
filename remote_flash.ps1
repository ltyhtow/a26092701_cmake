param(
    [string]$TargetIp = "100.79.218.27",
    [int]$Port = 33333
)

$elf = "build\debug_GCC_STM32C562CET6\a26092701.elf"

if (-not (Test-Path $elf)) {
    Write-Host "[-] ELF file not found: $elf. Building project first..." -ForegroundColor Yellow
    cmake --build --preset debug_GCC_STM32C562CET6
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed!"
        exit 1
    }
}

Write-Host ">>> [Remote Flash] Connecting to $TargetIp`:$Port ..." -ForegroundColor Cyan
& "C:\ST\STM32CubeCLT_1.20.0\GNU-tools-for-STM32\bin\arm-none-eabi-gdb.exe" --batch `
    -ex "target remote $TargetIp`:$Port" `
    -ex "monitor reset" `
    -ex "load" `
    -ex "monitor reset" `
    -ex "detach" `
    -ex "quit" `
    $elf

if ($LASTEXITCODE -eq 0) {
    Write-Host ">>> [SUCCESS] Firmware flashed and MCU reset successfully!" -ForegroundColor Green
} else {
    Write-Host ">>> [FAILED] Remote flash failed with exit code $LASTEXITCODE" -ForegroundColor Red
}
