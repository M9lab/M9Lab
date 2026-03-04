@echo off
setlocal enabledelayedexpansion

echo.
echo ========================================
echo   TrenIno - Conversione Automatica
echo ========================================
echo.
echo Converto tutti i file MP3 a 128 kbps CBR Mono...
echo.

REM Crea backup
if not exist "backup_originali" mkdir backup_originali

set count=0
set errors=0

for %%f in (*.mp3) do (
    set /a count+=1
    echo [!count!] %%f
    
    REM Backup
    copy "%%f" "backup_originali\%%f" >nul 2>&1
    
    REM Conversione
    ffmpeg -i "%%f" -ac 1 -b:a 128k -ar 44100 -acodec libmp3lame -y "temp_%%f" >nul 2>&1
    
    if !ERRORLEVEL! EQU 0 (
        move /y "temp_%%f" "%%f" >nul 2>&1
    ) else (
        set /a errors+=1
        if exist "temp_%%f" del "temp_%%f"
    )
)

echo.
echo ========================================
echo Completato: !count! file
echo Errori: !errors!
echo Backup: backup_originali\
echo ========================================
echo.
pause
