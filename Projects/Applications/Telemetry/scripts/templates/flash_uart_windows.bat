@echo off
setlocal

if "%~4"=="" (
  echo Usage: %~nx0 ^<firmware.hex^> ^<COMx^> ^<baud_rate^> ^<STM32_Programmer_CLI.exe path^>
  echo Example: %~nx0 ..\firmware\telemetry.hex COM5 115200 "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
  exit /b 1
)

set FW_HEX=%~1
set SERIAL_PORT=%~2
set BAUD=%~3
set STM32CLI=%~4

if not exist "%FW_HEX%" (
  echo Firmware file not found: %FW_HEX%
  exit /b 1
)

if not exist "%STM32CLI%" (
  echo STM32_Programmer_CLI not found: %STM32CLI%
  exit /b 1
)

"%STM32CLI%" -c port=%SERIAL_PORT% br=%BAUD% -w "%FW_HEX%" -v -rst
if errorlevel 1 exit /b 1

echo Flash complete via UART bootloader.
endlocal
