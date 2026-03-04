# ===================================================================
# Script Conversione MP3 per TrenIno
# Converte tutti i file MP3 a 128 kbps CBR Mono per audio ottimale
# ===================================================================

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  TrenIno - Conversione Audio MP3" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Verifica FFmpeg installato
$ffmpegPath = Get-Command ffmpeg -ErrorAction SilentlyContinue

if (-not $ffmpegPath) {
    Write-Host "[ERRORE] FFmpeg non trovato!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Installazione FFmpeg su Windows:" -ForegroundColor Yellow
    Write-Host "1. Scarica da: https://www.gyan.dev/ffmpeg/builds/" -ForegroundColor Yellow
    Write-Host "2. Prendi 'ffmpeg-release-essentials.zip'" -ForegroundColor Yellow
    Write-Host "3. Estrai in C:\ffmpeg" -ForegroundColor Yellow
    Write-Host "4. Aggiungi C:\ffmpeg\bin al PATH" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Oppure usa: winget install Gyan.FFmpeg" -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Premi Enter per uscire"
    exit 1
}

Write-Host "[OK] FFmpeg trovato: $($ffmpegPath.Source)" -ForegroundColor Green
Write-Host ""

# Crea cartella backup
$backupFolder = "backup_originali"
if (-not (Test-Path $backupFolder)) {
    New-Item -ItemType Directory -Path $backupFolder | Out-Null
    Write-Host "[INFO] Cartella backup creata: $backupFolder" -ForegroundColor Yellow
}

# Trova tutti i file MP3
$mp3Files = Get-ChildItem -Filter "*.mp3" -File

if ($mp3Files.Count -eq 0) {
    Write-Host "[ERRORE] Nessun file MP3 trovato!" -ForegroundColor Red
    Write-Host "Assicurati di eseguire lo script dalla cartella audio/" -ForegroundColor Red
    Write-Host ""
    Read-Host "Premi Enter per uscire"
    exit 1
}

Write-Host "[INFO] Trovati $($mp3Files.Count) file MP3" -ForegroundColor Cyan
Write-Host ""
Write-Host "Configurazione conversione:" -ForegroundColor Yellow
Write-Host "  - Bitrate: 128 kbps CBR (Constant Bit Rate)" -ForegroundColor Yellow
Write-Host "  - Canali: Mono (ideale per voce)" -ForegroundColor Yellow
Write-Host "  - Sample Rate: 44.1 kHz (standard)" -ForegroundColor Yellow
Write-Host "  - Encoder: LAME MP3 (alta qualita')" -ForegroundColor Yellow
Write-Host ""
Write-Host "NOTA: I file originali saranno spostati in backup_originali\" -ForegroundColor Magenta
Write-Host ""

$confirm = Read-Host "Vuoi continuare? (S/N)"
if ($confirm -ne "S" -and $confirm -ne "s") {
    Write-Host "Operazione annullata." -ForegroundColor Yellow
    exit 0
}

Write-Host ""
Write-Host "Inizio conversione..." -ForegroundColor Green
Write-Host ""

$converted = 0
$errors = 0
$totalSize = 0
$newSize = 0

foreach ($file in $mp3Files) {
    $fileName = $file.Name
    $originalSize = $file.Length
    
    Write-Host "Converto: $fileName " -NoNewline
    
    # Backup originale
    Copy-Item $file.FullName "$backupFolder\$fileName" -Force
    
    # File temporaneo
    $tempFile = "temp_$fileName"
    
    # Conversione: 128 kbps, CBR, Mono, 44.1kHz
    $ffmpegArgs = @(
        "-i", $file.FullName,
        "-ac", "1",              # Mono
        "-b:a", "128k",          # 128 kbps
        "-ar", "44100",          # 44.1 kHz
        "-acodec", "libmp3lame", # Encoder MP3
        "-y",                    # Sovrascrivi
        $tempFile
    )
    
    $process = Start-Process -FilePath "ffmpeg" -ArgumentList $ffmpegArgs -NoNewWindow -Wait -PassThru
    
    if ($process.ExitCode -eq 0 -and (Test-Path $tempFile)) {
        # Sostituisci originale
        Move-Item $tempFile $file.FullName -Force
        
        $newFileSize = (Get-Item $file.FullName).Length
        $totalSize += $originalSize
        $newSize += $newFileSize
        
        $sizeDiff = [math]::Round(($newFileSize - $originalSize) / 1024, 1)
        $percent = [math]::Round(($newFileSize / $originalSize) * 100, 0)
        
        if ($sizeDiff -gt 0) {
            Write-Host "[OK] +$($sizeDiff)KB ($percent%)" -ForegroundColor Green
        } else {
            Write-Host "[OK] $($sizeDiff)KB ($percent%)" -ForegroundColor Green
        }
        
        $converted++
    } else {
        Write-Host "[ERRORE]" -ForegroundColor Red
        $errors++
        if (Test-Path $tempFile) {
            Remove-Item $tempFile -Force
        }
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Conversione Completata" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "File convertiti: $converted" -ForegroundColor Green
if ($errors -gt 0) {
    Write-Host "Errori: $errors" -ForegroundColor Red
}

# Calcola risparmio spazio
if ($totalSize -gt 0) {
    $totalMB = [math]::Round($totalSize / 1MB, 2)
    $newMB = [math]::Round($newSize / 1MB, 2)
    $savedMB = [math]::Round(($totalSize - $newSize) / 1MB, 2)
    $savedPercent = [math]::Round((($totalSize - $newSize) / $totalSize) * 100, 0)
    
    Write-Host ""
    Write-Host "Dimensioni:" -ForegroundColor Cyan
    Write-Host "  Prima:  $totalMB MB" -ForegroundColor White
    Write-Host "  Dopo:   $newMB MB" -ForegroundColor White
    
    if ($savedMB -gt 0) {
        Write-Host "  Risparmiati: $savedMB MB ($savedPercent%)" -ForegroundColor Green
    } else {
        $increased = [math]::Abs($savedMB)
        Write-Host "  Aumento: $increased MB" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "Backup originali: backup_originali\" -ForegroundColor Yellow
Write-Host ""

if ($errors -gt 0) {
    Write-Host "[ATTENZIONE] Alcuni file hanno avuto errori!" -ForegroundColor Red
    Write-Host "Verifica backup_originali\ per recuperarli" -ForegroundColor Red
} else {
    Write-Host "Tutti i file convertiti con successo!" -ForegroundColor Green
}

Write-Host ""
Write-Host "Ora puoi copiare i file MP3 sulla SD Card!" -ForegroundColor Cyan
Write-Host ""

Read-Host "Premi Enter per uscire"
