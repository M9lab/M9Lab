@echo off
REM ===================================================================
REM Script Conversione MP3 per TrenIno
REM Converte tutti i file MP3 a 128 kbps CBR Mono per audio ottimale
REM ===================================================================

echo.
echo ========================================
echo   TrenIno - Conversione Audio MP3
echo ========================================
echo.

REM Verifica se FFmpeg e' installato
where ffmpeg >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERRORE] FFmpeg non trovato!
    echo.
    echo Installazione FFmpeg su Windows:
    echo 1. Scarica da: https://www.gyan.dev/ffmpeg/builds/
    echo 2. Prendi "ffmpeg-release-essentials.zip"
    echo 3. Estrai in C:\ffmpeg
    echo 4. Aggiungi C:\ffmpeg\bin al PATH
    echo.
    echo Oppure usa: winget install Gyan.FFmpeg
    echo.
    pause
    exit /b 1
)

echo [OK] FFmpeg trovato!
echo.

REM Crea cartella backup
if not exist "backup_originali" (
    mkdir backup_originali
    echo [INFO] Cartella backup creata
)

REM Conta file MP3
set count=0
for %%f in (*.mp3) do set /a count+=1

if %count%==0 (
    echo [ERRORE] Nessun file MP3 trovato nella cartella corrente!
    echo Assicurati di eseguire lo script dalla cartella audio/
    pause
    exit /b 1
)

echo [INFO] Trovati %count% file MP3
echo [INFO] Backup originali in: backup_originali\
echo.
echo Configurazione conversione:
echo   - Bitrate: 128 kbps CBR (Constant Bit Rate)
echo   - Canali: Mono (ideale per voce)
echo   - Sample Rate: 44.1 kHz (standard)
echo   - Qualita': Alta compressione MP3
echo.
echo NOTA: I file originali saranno spostati in backup_originali\
echo.
pause

echo.
echo Inizio conversione...
echo.

set converted=0
set errors=0

for %%f in (*.mp3) do (
    echo Converto: %%f
    
    REM Backup originale
    copy "%%f" "backup_originali\%%f" >nul 2>&1
    
    REM Conversione: 128 kbps, CBR, Mono, 44.1kHz
    ffmpeg -i "%%f" -ac 1 -b:a 128k -ar 44100 -acodec libmp3lame -y "temp_%%f" >nul 2>&1
    
    if %ERRORLEVEL% EQU 0 (
        REM Sostituisci originale con convertito
        move /y "temp_%%f" "%%f" >nul 2>&1
        echo   [OK] %%f
        set /a converted+=1
    ) else (
        echo   [ERRORE] %%f - conversione fallita
        set /a errors+=1
        if exist "temp_%%f" del "temp_%%f"
    )
)

echo.
echo ========================================
echo   Conversione Completata
echo ========================================
echo.
echo File convertiti: %converted%
echo Errori: %errors%
echo.
echo Backup originali: backup_originali\
echo.

if %errors% GTR 0 (
    echo [ATTENZIONE] Alcuni file hanno avuto errori!
    echo Verifica backup_originali\ per recuperarli
)

echo.
echo Ora puoi copiare i file MP3 sulla SD Card!
echo.
pause
