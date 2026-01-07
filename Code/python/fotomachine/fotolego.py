# -*- coding: utf-8 -*-
import tkinter as tk
import cv2
from PIL import Image, ImageTk, ImageDraw, ImageFont
import numpy as np
import time
import os
import urllib.request
from datetime import datetime
import smtplib
from email.message import EmailMessage
from dotenv import load_dotenv
import re  # Per validazione email
import platform  # Per rilevare sistema operativo

# ================= CARICAMENTO CONFIG DA .ENV =================
# Carica il file .env corretto in base al sistema operativo
system = platform.system()
if system == "Linux":
    # Raspberry Pi - usa .env.raspberry se esiste, altrimenti .env
    if os.path.exists(".env.raspberry"):
        load_dotenv(".env.raspberry")
        print("⚙️ Caricato: .env.raspberry (Raspberry Pi)")
    else:
        load_dotenv()
        print("⚙️ Caricato: .env (default)")
else:
    # Windows/Mac - usa .env standard
    load_dotenv()
    print("⚙️ Caricato: .env (Windows/Mac)")

# Dimensioni - Preview LIVE 16:9, Preview RESULT 4:3 ridotto, Foto finale 4:3
PREVIEW_LIVE_W = int(os.getenv("PREVIEW_LIVE_W", 1280))
PREVIEW_LIVE_H = int(os.getenv("PREVIEW_LIVE_H", 720))
PREVIEW_RESULT_W = int(os.getenv("PREVIEW_RESULT_W", 800))  # Ridotto per lasciare spazio ai controlli
PREVIEW_RESULT_H = int(os.getenv("PREVIEW_RESULT_H", 600))  # Ridotto per lasciare spazio ai controlli
PHOTO_W = int(os.getenv("PHOTO_W", 1366))
PHOTO_H = int(os.getenv("PHOTO_H", 1024))

# Risoluzione webcam per cattura (alta qualità)
WEBCAM_CAPTURE_W = int(os.getenv("WEBCAM_CAPTURE_W", 1920))
WEBCAM_CAPTURE_H = int(os.getenv("WEBCAM_CAPTURE_H", 1080))
COUNTDOWN_SECONDS = int(os.getenv("COUNTDOWN_SECONDS", 3))

# Cartelle input/output
INPUT_DIR = os.getenv("INPUT_DIR", "input_photos")
OUTPUT_DIR = os.getenv("OUTPUT_DIR", "output_photos")

# Variabile per testo evento
EVENT_TEXT = os.getenv("EVENT_TEXT", "Benvenuti al LEGO Museum! #LEGOTrains")

# Background template path
WINDOW_TEMPLATE_PATH = os.getenv("WINDOW_TEMPLATE_PATH", "background.png")

# Green screen - coordinate normalizzate per 1366x1024 (4:3)
# Con inclinazione yaw 0.5-1° (lato destro leggermente ruotato)
GS_TOP_LEFT_X = float(os.getenv("GS_TOP_LEFT_X", 0.4451))
GS_TOP_LEFT_Y = float(os.getenv("GS_TOP_LEFT_Y", 0.1787))
GS_TOP_RIGHT_X = float(os.getenv("GS_TOP_RIGHT_X", 0.9861))
GS_TOP_RIGHT_Y = float(os.getenv("GS_TOP_RIGHT_Y", 0.1820))  # Leggermente più basso (prospettiva)
GS_BOTTOM_RIGHT_X = float(os.getenv("GS_BOTTOM_RIGHT_X", 0.9861))
GS_BOTTOM_RIGHT_Y = float(os.getenv("GS_BOTTOM_RIGHT_Y", 0.6393))  # Leggermente più alto (prospettiva)
GS_BOTTOM_LEFT_X = float(os.getenv("GS_BOTTOM_LEFT_X", 0.4451))
GS_BOTTOM_LEFT_Y = float(os.getenv("GS_BOTTOM_LEFT_Y", 0.6426))

# Email config
SMTP_SERVER = os.getenv("SMTP_SERVER", "smtp.gmail.com")
SMTP_PORT = int(os.getenv("SMTP_PORT", 587))
SMTP_USER = os.getenv("SMTP_USER", "")
SMTP_PASSWORD = os.getenv("SMTP_PASSWORD", "")
EMAIL_SUBJECT = os.getenv("EMAIL_SUBJECT", "La tua foto LEGO").replace('\n', ' ').replace('\r', ' ').strip()

# HTML Email body semplificato - foto e social solo come link
EMAIL_HTML_BODY_DEFAULT = """<html>
<body style='font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px;'>
    <h2 style='color: #2C3E50; text-align: center;'>🎉 Grazie per aver visitato il nostro stand!</h2>
    <p style='font-size: 16px; line-height: 1.6;'>Trovi in allegato la tua <strong>foto della Maker Faire 2026</strong>! 📸</p>
    <p style='font-size: 16px; line-height: 1.6;'>Condividila sui social con gli hashtag:<br/>
    <strong style='color: #3498DB;'>#MFTS26 #M9LAB #fotoricordolego @legofotomachine</strong></p>
    <hr style='border: none; border-top: 2px solid #ddd; margin: 30px 0;'/>
    <h3 style='color: #3498DB; text-align: center;'>📱 Seguici sui nostri social!</h3>
    <p style='font-size: 15px; text-align: center; color: #555;'>
        Se ti è piaciuto il nostro stand allora <strong>lasciaci un LIKE</strong> sulle nostre pagine social! 👇
    </p>
    <table style='width: 100%; margin: 20px 0;'>
        <tr>
            <td style='text-align: center; padding: 20px;'>
                <a href='https://www.facebook.com/m9lab/' 
                   style='display: inline-block; margin: 15px 10px; background-color: #1877F2; color: white; 
                          padding: 14px 28px; text-decoration: none; border-radius: 8px; font-weight: bold; 
                          font-size: 15px; box-shadow: 0 3px 6px rgba(24,119,242,0.3);'>
                    👍 Facebook: M9Lab
                </a>
                <a href='https://www.instagram.com/mezzaninelab' 
                   style='display: inline-block; margin: 15px 10px; background-color: #E4405F; color: white; 
                          padding: 14px 28px; text-decoration: none; border-radius: 8px; font-weight: bold; 
                          font-size: 15px; box-shadow: 0 3px 6px rgba(228,64,95,0.3);'>
                    📸 Instagram: @mezzaninelab
                </a>
            </td>
        </tr>
    </table>
    <div style='background-color: #f8f9fa; padding: 20px; border-radius: 10px; text-align: center; margin-top: 30px;'>
        <p style='color: #2C3E50; font-size: 14px; margin: 0;'>Grazie per il tuo supporto! 🚀</p>
        <p style='color: #7f8c8d; font-size: 13px; margin: 10px 0 0 0;'>
            <strong>Team M9Lab</strong><br/>Lego Trains & Arduino Sketches
        </p>
    </div>
</body>
</html>"""

EMAIL_HTML_BODY = os.getenv("EMAIL_HTML_BODY", EMAIL_HTML_BODY_DEFAULT)

# ================= TESTI INTERFACCIA =================
# Bottoni
BTN_SHOOT_TEXT = os.getenv("BTN_SHOOT_TEXT", "SCATTA UNA FOTO")
BTN_CANCEL_TEXT = os.getenv("BTN_CANCEL_TEXT", "ANNULLA")
BTN_SEND_TEXT = os.getenv("BTN_SEND_TEXT", "INVIA FOTO")
BTN_EXIT_TEXT = os.getenv("BTN_EXIT_TEXT", "ESCI")

# Status messages
STATUS_READY = os.getenv("STATUS_READY", "Premi SCATTA UNA FOTO (o premi SPAZIO o INVIO)")
STATUS_ENTER_EMAIL = os.getenv("STATUS_ENTER_EMAIL", "Inserisci email e premi INVIO o TAB")
STATUS_CAMERA_ERROR = os.getenv("STATUS_CAMERA_ERROR", "Errore fotocamera")
STATUS_EMAIL_REQUIRED = os.getenv("STATUS_EMAIL_REQUIRED", "Inserisci un'email valida")
STATUS_EMAIL_INVALID = os.getenv("STATUS_EMAIL_INVALID", "Formato email non valido")
STATUS_EMAIL_SUCCESS = os.getenv("STATUS_EMAIL_SUCCESS", "Email inviata con successo!")
STATUS_EMAIL_ERROR = os.getenv("STATUS_EMAIL_ERROR", "Errore invio email: {error}")

# Entry placeholder
EMAIL_PLACEHOLDER = os.getenv("EMAIL_PLACEHOLDER", "Inserisci la tua email")

# Privacy notice
EMAIL_PRIVACY_NOTICE = os.getenv("EMAIL_PRIVACY_NOTICE", "Useremo la tua email solo per questa operazione. Niente archiviazione, niente spam.")

# ================= COLORI INTERFACCIA (TEMA M9LAB) =================
UI_BG_COLOR = os.getenv("UI_BG_COLOR", "#5DADE2")  # Azzurro M9Lab
UI_TEXT_COLOR = os.getenv("UI_TEXT_COLOR", "#FFFFFF")  # Bianco
UI_ACCENT_COLOR = os.getenv("UI_ACCENT_COLOR", "#FFD700")  # Giallo M9Lab
UI_LOGO_PATH = os.getenv("UI_LOGO_PATH", "logo.png")  # Path logo per pattern
UI_LOGO_OPACITY = float(os.getenv("UI_LOGO_OPACITY", 0.1))  # Opacità logo watermark

# ================= GLOBAL =================
MODE_PREVIEW = 0
MODE_RESULT = 1
mode = MODE_PREVIEW
current_photo_path = None
captured_frame = None  # Frame catturato prima del flash
inactivity_timer = None  # Timer per timeout automatico

# ================= CREAZIONE CARTELLE =================
os.makedirs(INPUT_DIR, exist_ok=True)
os.makedirs(OUTPUT_DIR, exist_ok=True)

# ================= CONFIGURAZIONE UI ADATTIVA =================
# Font sizes da .env (con fallback)
FONT_SIZE_COUNTDOWN = int(os.getenv("FONT_SIZE_COUNTDOWN", 200))
FONT_SIZE_STATUS = int(os.getenv("FONT_SIZE_STATUS", 24))
FONT_SIZE_BUTTON_MAIN = int(os.getenv("FONT_SIZE_BUTTON_MAIN", 26))
FONT_SIZE_BUTTON_SECONDARY = int(os.getenv("FONT_SIZE_BUTTON_SECONDARY", 20))
FONT_SIZE_EMAIL_ENTRY = int(os.getenv("FONT_SIZE_EMAIL_ENTRY", 24))
FONT_SIZE_EVENT_TEXT = int(os.getenv("FONT_SIZE_EVENT_TEXT", 60))

# Padding da .env (con fallback)
PADDING_FRAME = int(os.getenv("PADDING_FRAME", 20))
PADDING_BUTTON = int(os.getenv("PADDING_BUTTON", 20))

# Dimensioni UI engagement screen
QR_CODE_SIZE = int(os.getenv("QR_CODE_SIZE", 350))
FONT_SIZE_ENGAGEMENT_TITLE = int(os.getenv("FONT_SIZE_ENGAGEMENT_TITLE", 36))
FONT_SIZE_ENGAGEMENT_TEXT = int(os.getenv("FONT_SIZE_ENGAGEMENT_TEXT", 28))
FONT_SIZE_ENGAGEMENT_SOCIAL = int(os.getenv("FONT_SIZE_ENGAGEMENT_SOCIAL", 24))
FONT_SIZE_ENGAGEMENT_THANKS = int(os.getenv("FONT_SIZE_ENGAGEMENT_THANKS", 20))

# Font emoji - Linux usa Noto Color Emoji, Windows usa Segoe UI Emoji
if system == "Linux":
    EMOJI_FONT = "Noto Color Emoji"
else:
    EMOJI_FONT = "Segoe UI Emoji"

# ================= WEBCAM =================
# Usa il backend corretto (system già definito sopra)
if system == "Windows":
    cap = cv2.VideoCapture(0, cv2.CAP_DSHOW)
    print("Sistema: Windows - Usando backend CAP_DSHOW")
elif system == "Linux":
    cap = cv2.VideoCapture(0, cv2.CAP_V4L2)
    print("Sistema: Linux - Usando backend CAP_V4L2")
else:
    cap = cv2.VideoCapture(0)  # Default per macOS e altri
    print(f"Sistema: {system} - Usando backend default")

if not cap.isOpened():
    raise RuntimeError("Errore apertura fotocamera. Verifica che la fotocamera sia connessa e abilitata.")

# Imposta risoluzione webcam alta per migliore qualità
cap.set(cv2.CAP_PROP_FRAME_WIDTH, WEBCAM_CAPTURE_W)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, WEBCAM_CAPTURE_H)

# Verifica risoluzione effettiva
actual_w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
actual_h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

# Carica background per info
if os.path.exists(WINDOW_TEMPLATE_PATH):
    bg_temp = cv2.imread(WINDOW_TEMPLATE_PATH)
    bg_h, bg_w = bg_temp.shape[:2]
    print(f"Background: {bg_w}x{bg_h}")
else:
    print("Nessun background trovato")

print(f"Webcam: {actual_w}x{actual_h}")
print(f"Preview LIVE: {PREVIEW_LIVE_W}x{PREVIEW_LIVE_H} (16:9)")
print(f"Preview RESULT: {PREVIEW_RESULT_W}x{PREVIEW_RESULT_H} (4:3)")
print(f"Output finale: {PHOTO_W}x{PHOTO_H} (4:3)")

# ================= FUNZIONI =================
def resize_no_crop(image, target_size):
    """Ridimensiona l'immagine senza crop, mantenendo tutto visibile"""
    return image.resize(target_size, Image.LANCZOS)

def update_preview():
    if mode == MODE_PREVIEW:
        ret, frame = cap.read()
        if ret:
            # Preview LIVE - Ridimensiona webcam 16:9 direttamente a 16:9 (no barre nere)
            frame_resized = cv2.resize(frame, (PREVIEW_LIVE_W, PREVIEW_LIVE_H), interpolation=cv2.INTER_LINEAR)
            img = Image.fromarray(cv2.cvtColor(frame_resized, cv2.COLOR_BGR2RGB))
            
            # Disegna countdown sull'immagine se attivo
            if hasattr(root, 'countdown_active') and root.countdown_active:
                draw = ImageDraw.Draw(img)
                text = str(root.countdown_number)
                # Font grande per countdown (dimensione da .env)
                try:
                    font = ImageFont.truetype("arial.ttf", FONT_SIZE_COUNTDOWN)
                except:
                    try:
                        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", FONT_SIZE_COUNTDOWN)
                    except:
                        font = ImageFont.load_default()
                
                # Calcola posizione centrata
                bbox = draw.textbbox((0, 0), text, font=font)
                text_w = bbox[2] - bbox[0]
                text_h = bbox[3] - bbox[1]
                x = (PREVIEW_LIVE_W - text_w) // 2
                y = (PREVIEW_LIVE_H - text_h) // 2
                
                # Disegna outline nero per contrasto
                outline_width = 8
                for adj_x in range(-outline_width, outline_width+1, 4):
                    for adj_y in range(-outline_width, outline_width+1, 4):
                        draw.text((x+adj_x, y+adj_y), text, font=font, fill="black")
                
                # Disegna testo principale giallo
                draw.text((x, y), text, font=font, fill=UI_ACCENT_COLOR)
            
            imgtk = ImageTk.PhotoImage(img)
            preview_label.imgtk = imgtk
            preview_label.config(image=imgtk)
    root.after(50, update_preview)

def start_countdown():
    btn_shoot.config(state="disabled")
    root.countdown_active = True
    root.countdown_number = COUNTDOWN_SECONDS
    countdown_step(COUNTDOWN_SECONDS)

def countdown_step(sec):
    if sec > 0:
        root.countdown_number = sec
        root.after(1000, lambda: countdown_step(sec - 1))
    else:
        root.countdown_active = False
        # CATTURA IMMEDIATA + FLASH (ottimizzato per Raspberry)
        capture_and_flash()

def capture_and_flash():
    """Cattura il frame SUBITO, poi mostra flash come effetto visivo"""
    global captured_frame
    
    # PASSO 1: CATTURA IMMEDIATA del frame (prima del flash!)
    # Su Raspberry, flush del buffer per ottenere frame fresco
    for _ in range(3):  # Ridotto a 3 per essere più veloce
        cap.read()
    
    ret, captured_frame = cap.read()
    if not ret:
        status_label.config(text=STATUS_CAMERA_ERROR, fg="#FFFFFF", bg="#E74C3C")
        btn_shoot.config(state="normal")
        return
    
    # PASSO 2: Mostra flash DOPO aver catturato (effetto visivo)
    flash_effect()

def flash_effect():
    """Effetto flash bianco DOPO lo scatto (solo effetto visivo)"""
    # Crea overlay bianco a schermo intero
    flash_label = tk.Label(root, bg="white", borderwidth=0)
    flash_label.place(x=0, y=0, relwidth=1, relheight=1)
    
    # Fade out del flash (veloce)
    def fade_flash(alpha=1.0):
        if alpha > 0:
            # Calcola colore che va da bianco a trasparente
            gray_val = int(255 * alpha)
            color = f'#{gray_val:02x}{gray_val:02x}{gray_val:02x}'
            flash_label.config(bg=color)
            root.after(30, lambda: fade_flash(alpha - 0.25))  # Più veloce
        else:
            flash_label.place_forget()
            flash_label.destroy()
            # Processa la foto già catturata
            process_captured_photo()
    
    # Inizia il fade out immediatamente
    root.after(10, lambda: fade_flash(1.0))

def process_captured_photo():
    """Processa il frame già catturato (chiamato dopo il flash)"""
    global mode, current_photo_path, captured_frame
    
    # Usa il frame già catturato
    frame = captured_frame

    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    input_path = os.path.join(INPUT_DIR, f"{ts}.png")
    output_path = os.path.join(OUTPUT_DIR, f"{ts}.jpg")

    # Salva foto originale senza crop
    cv2.imwrite(input_path, frame)

    # ================= COMPOSIZIONE CON GREEN SCREEN =================
    if os.path.exists(WINDOW_TEMPLATE_PATH):
        # Carica background
        background = cv2.imread(WINDOW_TEMPLATE_PATH)
        bg_h, bg_w = background.shape[:2]
        
        # Calcola coordinate assolute dei 4 angoli del green screen
        pts_tl = np.array([GS_TOP_LEFT_X * bg_w, GS_TOP_LEFT_Y * bg_h])
        pts_tr = np.array([GS_TOP_RIGHT_X * bg_w, GS_TOP_RIGHT_Y * bg_h])
        pts_br = np.array([GS_BOTTOM_RIGHT_X * bg_w, GS_BOTTOM_RIGHT_Y * bg_h])
        pts_bl = np.array([GS_BOTTOM_LEFT_X * bg_w, GS_BOTTOM_LEFT_Y * bg_h])
        
        dst_pts = np.float32([pts_tl, pts_tr, pts_br, pts_bl])
        
        # Calcola dimensioni reali del green screen (differenza coordinate)
        gs_width = int(pts_tr[0] - pts_tl[0])
        gs_height = int(pts_bl[1] - pts_tl[1])
        
        print(f"Green screen reale: {gs_width}x{gs_height} px (ratio {gs_width/gs_height:.2f})")
        print(f"Frame catturato: {frame.shape[1]}x{frame.shape[0]} px")
        
        # Ridimensiona la foto per coprire esattamente il green screen
        # Aggiungi un piccolo fattore di scala per coprire completamente
        scale_factor = 1.02
        target_width = int(gs_width * scale_factor)
        target_height = int(gs_height * scale_factor)
        
        frame_resized = cv2.resize(frame, (target_width, target_height), interpolation=cv2.INTER_LANCZOS4)
        
        print(f"Frame ridimensionato: {frame_resized.shape[1]}x{frame_resized.shape[0]} px")
        
        # Coordinate sorgente: usiamo l'intera immagine ridimensionata
        src_pts = np.float32([
            [0, 0],                           # Top-Left
            [target_width - 1, 0],            # Top-Right
            [target_width - 1, target_height - 1],  # Bottom-Right
            [0, target_height - 1]            # Bottom-Left
        ])
        
        # Calcola matrice di trasformazione prospettica
        # Mappa la foto rettangolare sul quadrilatero del green screen
        matrix = cv2.getPerspectiveTransform(src_pts, dst_pts)
        
        # Applica warp sulla foto
        warped = cv2.warpPerspective(frame_resized, matrix, (bg_w, bg_h), 
                                     flags=cv2.INTER_LANCZOS4,
                                     borderMode=cv2.BORDER_CONSTANT,
                                     borderValue=(0, 0, 0))
        
        # ================= RILEVAMENTO GREEN SCREEN ULTRA-PRECISO =================
        # Converti background in HSV
        hsv_bg = cv2.cvtColor(background, cv2.COLOR_BGR2HSV)
        
        # Range MOLTO preciso per il verde - solo verde puro brillante
        # Hue: 35-75 (verde), Saturation: 80+ (molto saturo), Value: 80+ (brillante)
        lower_green = np.array([35, 80, 80])    # Verde MOLTO saturo e brillante
        upper_green = np.array([75, 255, 255])   # Range stretto
        
        # Crea maschera del green screen con range molto stretto
        green_mask = cv2.inRange(hsv_bg, lower_green, upper_green)
        
        # Morphological operations più conservative
        kernel_close = np.ones((5, 5), np.uint8)
        kernel_open = np.ones((3, 3), np.uint8)
        
        # Chiudi solo piccoli buchi
        green_mask = cv2.morphologyEx(green_mask, cv2.MORPH_CLOSE, kernel_close, iterations=1)
        
        # Rimuovi piccoli artefatti
        green_mask = cv2.morphologyEx(green_mask, cv2.MORPH_OPEN, kernel_open, iterations=1)
        
        # NON dilatare troppo - vogliamo essere conservativi per non coprire elementi
        kernel_dilate = np.ones((3, 3), np.uint8)
        green_mask = cv2.dilate(green_mask, kernel_dilate, iterations=1)
        
        # Sfuma SOLO leggermente i bordi
        green_mask = cv2.GaussianBlur(green_mask, (5, 5), 0)
        
        # Crea maschera del poligono green screen ma NON dilatarla troppo
        poly_mask = np.zeros((bg_h, bg_w), dtype=np.uint8)
        cv2.fillConvexPoly(poly_mask, dst_pts.astype(np.int32), 255)
        
        # NON dilatare il poligono - mantieni i bordi precisi
        # Così gli elementi 3D ai bordi non vengono coperti
        
        # Combina le maschere: usa SOLO pixel che sono sia verdi CHE dentro il poligono
        # Questo esclude automaticamente oggetti non verdi come la paletta
        combined_mask = cv2.bitwise_and(green_mask, poly_mask)
        
        # Applica erosione finale per essere SICURI di non coprire elementi in primo piano
        kernel_erode = np.ones((2, 2), np.uint8)
        combined_mask = cv2.erode(combined_mask, kernel_erode, iterations=1)
        
        # Normalizza maschera per blending
        mask_3ch = cv2.cvtColor(combined_mask, cv2.COLOR_GRAY2BGR).astype(float) / 255.0
        
        # Blending finale - SOLO dove c'è verde puro
        final_cv = (background.astype(float) * (1 - mask_3ch) + 
                    warped.astype(float) * mask_3ch).astype(np.uint8)
        
        # Converti in PIL per aggiungere testo
        final_image = Image.fromarray(cv2.cvtColor(final_cv, cv2.COLOR_BGR2RGB))
    else:
        # Nessun background, ridimensiona la foto al formato finale
        frame_resized = cv2.resize(frame, (PHOTO_W, PHOTO_H), interpolation=cv2.INTER_LANCZOS4)
        final_image = Image.fromarray(cv2.cvtColor(frame_resized, cv2.COLOR_BGR2RGB))

    # ================= TESTO EVENTO CON SFONDO =================
    draw = ImageDraw.Draw(final_image)
    try:
        font = ImageFont.truetype("arialbd.ttf", FONT_SIZE_EVENT_TEXT)
    except:
        try:
            font = ImageFont.truetype("arial.ttf", FONT_SIZE_EVENT_TEXT)
        except:
            try:
                font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", FONT_SIZE_EVENT_TEXT)
            except:
                font = ImageFont.load_default()
    
    # Calcola bounding box del testo (coordinate relative)
    bbox = draw.textbbox((0, 0), EVENT_TEXT, font=font)
    text_w = bbox[2] - bbox[0]
    text_h = bbox[3] - bbox[1]
    
    # Posiziona il testo in base alla dimensione dell'immagine finale
    final_w, final_h = final_image.size
    text_x = (final_w - text_w) // 2
    text_y = final_h - 120
    
    # Disegna rettangolo di sfondo semi-trasparente
    # Padding uniforme che include anche l'outline width
    outline_width = 3
    padding_h = 25  # Padding orizzontale
    padding_v = 20  # Padding verticale
    
    # Calcola rettangolo considerando anche l'outline
    bg_rect = [
        text_x - padding_h - outline_width,
        text_y - padding_v - outline_width + bbox[1],  # Aggiusta per offset baseline
        text_x + text_w + padding_h + outline_width,
        text_y + text_h + padding_v + outline_width + bbox[1]
    ]
    
    # Crea layer semi-trasparente per sfondo
    overlay = Image.new('RGBA', final_image.size, (0, 0, 0, 0))
    overlay_draw = ImageDraw.Draw(overlay)
    overlay_draw.rectangle(bg_rect, fill=(0, 0, 0, 180))  # Nero semi-trasparente
    
    # Converti final_image a RGBA se necessario
    if final_image.mode != 'RGBA':
        final_image = final_image.convert('RGBA')
    
    # Applica overlay
    final_image = Image.alpha_composite(final_image, overlay)
    
    # Ridisegna il draw object dopo la conversione
    draw = ImageDraw.Draw(final_image)
    
    # Disegna outline nero per maggior contrasto (usa outline_width già definito sopra)
    for adj_x in range(-outline_width, outline_width+1):
        for adj_y in range(-outline_width, outline_width+1):
            if adj_x != 0 or adj_y != 0:
                draw.text((text_x + adj_x, text_y + adj_y), EVENT_TEXT, fill="black", font=font)
    
    # Disegna testo principale giallo brillante
    draw.text((text_x, text_y), EVENT_TEXT, fill="#FFD700", font=font)
    
    # Riconverti a RGB per salvare come JPEG
    final_image = final_image.convert('RGB')

    # Salva JPEG compressa ma alta qualità
    final_image.save(output_path, format="JPEG", quality=95)
    current_photo_path = output_path
    show_result(final_image)

def show_result(image_pil):
    global mode
    mode = MODE_RESULT
    
    # Mostra foto composta 4:3 nell'anteprima
    imgtk = ImageTk.PhotoImage(image_pil.resize((PREVIEW_RESULT_W, PREVIEW_RESULT_H)))
    preview_label.imgtk = imgtk
    preview_label.config(image=imgtk)
    
    # IMPORTANTE: Rimuovi tutto prima di riorganizzare
    status_frame.pack_forget()
    preview_frame.pack_forget()
    btn_frame.pack_forget()
    entry_frame.pack_forget()
    btn_send.pack_forget()
    
    # Nascondi privacy label e bottoni azione se esistevano
    if hasattr(root, 'privacy_label') and root.privacy_label and root.privacy_label.winfo_exists():
        root.privacy_label.pack_forget()
    if hasattr(root, 'btn_action_frame') and root.btn_action_frame and root.btn_action_frame.winfo_exists():
        root.btn_action_frame.pack_forget()
    
    # Ordine corretto: FOTO → LABEL ISTRUZIONI → CAMPO EMAIL → BOTTONE
    
    # 1. Foto in alto
    preview_frame.pack(expand=False, pady=PADDING_FRAME, padx=PADDING_FRAME)
    
    # 2. Label con istruzioni sopra il campo email
    status_frame.pack(pady=(PADDING_FRAME, 0))  # Padding solo sopra
    status_label.config(text=STATUS_ENTER_EMAIL, fg="#FFFFFF", bg="#2C3E50")
    
    # 3. Campo email
    entry_frame.pack(pady=(5, 0))  # Poco padding sopra, zero sotto (privacy notice sotto)
    entry_email.pack(padx=40, pady=PADDING_FRAME, ipady=8)
    
    # 3b. Label privacy sotto il campo email in giallo M9Lab (su una riga)
    privacy_label = tk.Label(root, text=EMAIL_PRIVACY_NOTICE, 
                            font=("Arial", 14, "italic"),  # Font aumentato da 11 a 14
                            fg=UI_ACCENT_COLOR,  # Giallo M9Lab (#FFD700)
                            bg=UI_BG_COLOR)  # Nessun wrap - testo su una riga
    privacy_label.pack(pady=(0, PADDING_FRAME))
    
    # 4. Frame per bottoni INVIA e ANNULLA affiancati
    btn_action_frame = tk.Frame(root, bg=UI_BG_COLOR)
    btn_action_frame.pack(pady=PADDING_FRAME)
    
    # Salva riferimenti per poterli distruggere nel reset
    root.btn_action_frame = btn_action_frame
    root.privacy_label = privacy_label
    
    # Crea bottone INVIA FOTO a sinistra (principale)
    btn_send_result = tk.Button(btn_action_frame, text=BTN_SEND_TEXT, 
                                 font=("Arial", FONT_SIZE_BUTTON_MAIN, "bold"), 
                                 bg="#2196F3", fg="white", 
                                 activebackground="#1976D2", activeforeground="white",
                                 width=20, height=1, pady=5,
                                 command=send_email, 
                                 relief=tk.FLAT, borderwidth=0, cursor="hand2",
                                 takefocus=True)  # Navigabile con TAB
    btn_send_result.grid(row=0, column=0, padx=5)
    
    # Crea bottone ANNULLA a destra (secondario) - stessa altezza di INVIA
    btn_cancel_result = tk.Button(btn_action_frame, text=BTN_CANCEL_TEXT, 
                                   font=("Arial", FONT_SIZE_BUTTON_MAIN, "bold"),  # Stesso font di INVIA
                                   bg="#757575", fg="white", 
                                   activebackground="#616161", activeforeground="white",
                                   width=20, height=1, pady=5,  # Stessa width di INVIA
                                   command=reset_kiosk, 
                                   relief=tk.FLAT, borderwidth=0, cursor="hand2",
                                   takefocus=True)  # Navigabile con TAB
    btn_cancel_result.grid(row=0, column=1, padx=5)
    
    # Imposta ordine TAB corretto: EMAIL → INVIA → ANNULLA → EXIT
    # Forza il focus su INVIA quando si preme TAB dall'email
    def email_tab_handler(event):
        btn_send_result.focus_set()
        return "break"  # Previene comportamento default
    
    entry_email.bind('<Tab>', email_tab_handler)
    
    # TAB da INVIA va ad ANNULLA
    def send_tab_handler(event):
        btn_cancel_result.focus_set()
        return "break"
    
    btn_send_result.bind('<Tab>', send_tab_handler)
    
    # TAB da ANNULLA va ad EXIT
    def cancel_tab_handler(event):
        btn_exit.focus_set()
        return "break"
    
    btn_cancel_result.bind('<Tab>', cancel_tab_handler)
    
    # Nascondi bottoni originali
    btn_frame.pack_forget()
    btn_cancel.grid_forget()
    btn_send.pack_forget()
    
    # Resetta campo email con placeholder semplice
    entry_email.delete(0, tk.END)
    entry_email.config(fg="#999999", highlightbackground="#E0E0E0", highlightcolor="#4CAF50", highlightthickness=3)
    entry_email.insert(0, EMAIL_PLACEHOLDER)
    
    # Focus automatico sul campo email per iniziare a scrivere (rimuove placeholder)
    root.after(100, lambda: entry_email.focus_set())
    # Avvia timer di 30 secondi per reset automatico
    start_inactivity_timer()

def show_engagement_screen():
    """Mostra schermata engagement con QR code dopo invio email"""
    global mode
    mode = "ENGAGEMENT"  # Nuovo stato per gestire i tasti
    
    # Nascondi tutto
    preview_frame.pack_forget()
    entry_frame.pack_forget()
    entry_email.pack_forget()
    btn_send.pack_forget()
    btn_cancel.grid_forget()
    status_frame.pack_forget()
    btn_frame.pack_forget()
    
    # Nascondi privacy label e bottoni azione se esistono
    if hasattr(root, 'privacy_label') and root.privacy_label and root.privacy_label.winfo_exists():
        root.privacy_label.pack_forget()
    if hasattr(root, 'btn_action_frame') and root.btn_action_frame and root.btn_action_frame.winfo_exists():
        root.btn_action_frame.pack_forget()
    
    # Crea frame engagement
    engagement_frame = tk.Frame(root, bg=UI_BG_COLOR)
    engagement_frame.pack(expand=True, fill=tk.BOTH)
    
    # Titolo successo (frame per allineamento emoji)
    title_frame = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    title_frame.pack(pady=30)
    
    emoji_success = tk.Label(title_frame, text="✅", font=(EMOJI_FONT, FONT_SIZE_ENGAGEMENT_TITLE), 
                            fg="#FFFFFF", bg=UI_BG_COLOR)
    emoji_success.pack(side=tk.LEFT, padx=(0, 10))
    
    success_title = tk.Label(title_frame, 
                            text="Email inviata con successo!", 
                            font=("Arial", FONT_SIZE_ENGAGEMENT_TITLE, "bold"), 
                            fg="#FFFFFF", bg=UI_BG_COLOR)
    success_title.pack(side=tk.LEFT)
    
    # Carica e mostra QR code Facebook con dimensione da .env
    qr_code_path = "m9lab_facebook_qrcode.png"
    if os.path.exists(qr_code_path):
        try:
            qr_img = Image.open(qr_code_path)
            qr_img = qr_img.resize((QR_CODE_SIZE, QR_CODE_SIZE), Image.LANCZOS)
            qr_tk = ImageTk.PhotoImage(qr_img)
            qr_label = tk.Label(engagement_frame, image=qr_tk, bg=UI_BG_COLOR, 
                               borderwidth=6, relief=tk.SOLID, bd=6)
            qr_label.image = qr_tk  # Mantieni riferimento
            qr_label.pack(pady=20)
        except:
            pass
    
    # Testo invito (frame per allineamento emoji)
    invite_frame = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    invite_frame.pack(pady=20)
    
    emoji_phone = tk.Label(invite_frame, text="📱", font=(EMOJI_FONT, FONT_SIZE_ENGAGEMENT_TEXT), 
                          fg=UI_TEXT_COLOR, bg=UI_BG_COLOR)
    emoji_phone.pack(side=tk.LEFT, padx=(0, 10))
    
    invite_text = tk.Label(invite_frame, 
                           text="Scannerizza il QR code\no seguici sui social!", 
                           font=("Arial", FONT_SIZE_ENGAGEMENT_TEXT, "bold"), 
                           fg=UI_TEXT_COLOR, bg=UI_BG_COLOR,
                           justify=tk.LEFT)
    invite_text.pack(side=tk.LEFT)
    
    # Link social (frame per allineamento emoji)
    social_frame_links = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    social_frame_links.pack(pady=15)
    
    emoji_fb = tk.Label(social_frame_links, text="👍", font=(EMOJI_FONT, FONT_SIZE_ENGAGEMENT_SOCIAL), 
                       fg="#FFD700", bg=UI_BG_COLOR)
    emoji_fb.pack(side=tk.LEFT, padx=(0, 5))
    
    fb_text = tk.Label(social_frame_links, text="Facebook: M9Lab", 
                      font=("Arial", FONT_SIZE_ENGAGEMENT_SOCIAL), fg="#FFD700", bg=UI_BG_COLOR)
    fb_text.pack(side=tk.LEFT, padx=(0, 20))
    
    separator = tk.Label(social_frame_links, text="|", 
                        font=("Arial", FONT_SIZE_ENGAGEMENT_SOCIAL), fg="#FFD700", bg=UI_BG_COLOR)
    separator.pack(side=tk.LEFT, padx=10)
    
    emoji_ig = tk.Label(social_frame_links, text="📸", font=(EMOJI_FONT, FONT_SIZE_ENGAGEMENT_SOCIAL), 
                       fg="#FFD700", bg=UI_BG_COLOR)
    emoji_ig.pack(side=tk.LEFT, padx=(0, 5))
    
    ig_text = tk.Label(social_frame_links, text="Instagram: @mezzaninelab", 
                      font=("Arial", FONT_SIZE_ENGAGEMENT_SOCIAL), fg="#FFD700", bg=UI_BG_COLOR)
    ig_text.pack(side=tk.LEFT)
    
    # Messaggio grazie (frame per allineamento emoji)
    thanks_frame = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    thanks_frame.pack(pady=20)
    
    thanks_text = tk.Label(thanks_frame, 
                          text="Grazie per aver visitato il nostro stand!", 
                          font=("Arial", FONT_SIZE_ENGAGEMENT_THANKS), 
                          fg=UI_TEXT_COLOR, bg=UI_BG_COLOR)
    thanks_text.pack(side=tk.LEFT, padx=(0, 10))
    
    emoji_rocket = tk.Label(thanks_frame, text="🚀", font=(EMOJI_FONT, FONT_SIZE_ENGAGEMENT_THANKS), 
                           fg=UI_TEXT_COLOR, bg=UI_BG_COLOR)
    emoji_rocket.pack(side=tk.LEFT)
    
    # Bottone per scattare un'altra foto (frame per allineamento emoji)
    btn_frame_another = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    btn_frame_another.pack(pady=30)
    
    btn_another = tk.Button(btn_frame_another, 
                            text="  SCATTA UN'ALTRA FOTO", 
                            font=("Arial", FONT_SIZE_ENGAGEMENT_SOCIAL, "bold"), 
                            bg="#4CAF50", fg="white", 
                            activebackground="#45a049", activeforeground="white",
                            width=28, height=2,
                            command=reset_kiosk,
                            relief=tk.FLAT, borderwidth=0,
                            cursor="hand2",
                            compound=tk.LEFT)
    
    # Aggiungi emoji come compound left (Tkinter gestisce meglio così)
    try:
        # Crea una piccola label per emoji sovrapposta
        btn_another.config(text="📸  SCATTA UN'ALTRA FOTO")
    except:
        btn_another.config(text="  SCATTA UN'ALTRA FOTO")
    
    btn_another.pack()
    btn_another.focus_set()  # Focus sul bottone
    
    # Istruzioni
    instruction_text = tk.Label(engagement_frame, 
                               text="Premi SPAZIO o INVIO per continuare", 
                               font=("Arial", 16), 
                               fg="#FFFFFF", bg=UI_BG_COLOR)
    instruction_text.pack(pady=10)
    
    # Salva riferimento
    root.engagement_frame = engagement_frame
    
    # Timer 30 secondi prima di tornare all'inizio (salvato per poterlo cancellare)
    root.engagement_timer = root.after(30000, reset_kiosk)

def reset_kiosk():
    global mode, inactivity_timer
    
    # Cancella timer inattività se attivo
    if inactivity_timer:
        root.after_cancel(inactivity_timer)
        inactivity_timer = None
    
    # Cancella timer engagement se attivo
    if hasattr(root, 'engagement_timer') and root.engagement_timer:
        root.after_cancel(root.engagement_timer)
        root.engagement_timer = None
    
    mode = MODE_PREVIEW
    
    # Nascondi frame engagement se esiste
    if hasattr(root, 'engagement_frame'):
        root.engagement_frame.pack_forget()
    
    # Nascondi elementi email (anche privacy label e bottoni azione)
    entry_frame.pack_forget()
    entry_email.pack_forget()
    btn_send.pack_forget()
    btn_cancel.grid_forget()
    
    # Nascondi privacy label e bottoni azione se esistono
    if hasattr(root, 'privacy_label') and root.privacy_label and root.privacy_label.winfo_exists():
        root.privacy_label.pack_forget()
    if hasattr(root, 'btn_action_frame') and root.btn_action_frame and root.btn_action_frame.winfo_exists():
        root.btn_action_frame.pack_forget()
    
    # Distruggi elementi dinamici (DOPO aver gestito gli altri elementi)
    def destroy_dynamic_elements():
        # Distruggi frame bottoni
        if hasattr(root, 'btn_action_frame') and root.btn_action_frame:
            try:
                root.btn_action_frame.destroy()
            except:
                pass
            root.btn_action_frame = None
        
        # Distruggi label privacy
        if hasattr(root, 'privacy_label') and root.privacy_label:
            try:
                root.privacy_label.destroy()
            except:
                pass
            root.privacy_label = None
    
    root.after(10, destroy_dynamic_elements)
    
    # Mostra elementi standard (stesso ordine dell'avvio)
    preview_frame.pack(expand=True, pady=PADDING_BUTTON, padx=PADDING_BUTTON)
    status_frame.pack(pady=PADDING_FRAME)
    btn_frame.pack(pady=PADDING_BUTTON)
    
    btn_shoot.grid(row=0, column=0)
    status_label.config(text=STATUS_READY, fg="#FFFFFF", bg="#2C3E50")
    btn_shoot.config(state="normal")
    # Ripristina focus sul bottone scatta
    root.after(100, lambda: btn_shoot.focus_set())

def start_inactivity_timer():
    """Avvia timer di 30 secondi per reset automatico"""
    global inactivity_timer
    
    # Cancella timer precedente se esiste
    if inactivity_timer:
        root.after_cancel(inactivity_timer)
    
    # Avvia nuovo timer di 30 secondi
    inactivity_timer = root.after(30000, auto_reset_on_timeout)

def auto_reset_on_timeout():
    """Reset automatico dopo 30 secondi di inattività"""
    global inactivity_timer
    inactivity_timer = None
    
    if mode == MODE_RESULT:
        status_label.config(text="Timeout - Ritorno alla schermata iniziale...", fg="#FFFFFF", bg="#FF9800")
        root.after(1000, reset_kiosk)

def reset_inactivity_timer(event=None):
    """Reset del timer quando c'è attività"""
    if mode == MODE_RESULT:
        start_inactivity_timer()

def validate_email(email):
    """
    Valida il formato dell'email usando regex.
    Returns: True se valida, False altrimenti
    """
    # Pattern RFC 5322 semplificato ma robusto
    email_pattern = r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
    
    # Controlla se l'email corrisponde al pattern
    if re.match(email_pattern, email):
        # Controlli aggiuntivi
        if '..' in email:  # No doppi punti consecutivi
            return False
        if email.startswith('.') or email.startswith('-'):  # No inizio con . o -
            return False
        if '@.' in email or '.@' in email:  # No punto vicino a @
            return False
        return True
    return False

def log_email_send(esito, email_address, photo_filename):
    """
    Registra l'invio email in un file di log.
    
    Args:
        esito: "OK" per successo, "KO" per fallimento
        email_address: Indirizzo email destinatario
        photo_filename: Nome del file foto allegato
    """
    log_file = "email_log.txt"
    
    try:
        # Calcola il contatore leggendo il numero di righe esistenti
        contatore = 1
        if os.path.exists(log_file):
            with open(log_file, "r", encoding="utf-8") as f:
                contatore = sum(1 for line in f) + 1
        
        # Timestamp formato: YYYY-MM-DD HH:MM:SS
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        # Scrivi la riga di log: contatore | timestamp | esito | nome_foto | email
        with open(log_file, "a", encoding="utf-8") as f:
            f.write(f"{contatore} | {timestamp} | {esito} | {photo_filename} | {email_address}\n")
        
        print(f"📝 Log #{contatore}: {esito} - {photo_filename} - {email_address}")
        
    except Exception as e:
        print(f"⚠️ Errore scrittura log: {e}")

def send_email():
    email_to = entry_email.get().strip()
    
    # Controlla se l'email è vuota o è ancora il placeholder
    if not email_to or email_to == EMAIL_PLACEHOLDER:
        status_label.config(text=STATUS_EMAIL_REQUIRED, fg="#FFFFFF", bg="#E74C3C")
        entry_email.config(highlightbackground="#FF6B6B", highlightcolor="#FF6B6B", highlightthickness=2)
        return
    
    # Valida il formato dell'email
    if not validate_email(email_to):
        status_label.config(text=STATUS_EMAIL_INVALID, fg="#FFFFFF", bg="#E74C3C")
        entry_email.config(highlightbackground="#FF6B6B", highlightcolor="#FF6B6B", highlightthickness=2)
        return
    
    # Email valida - Nascondi immediatamente il campo email, privacy label e i bottoni
    entry_frame.pack_forget()
    entry_email.pack_forget()
    btn_send.pack_forget()
    btn_cancel.grid_forget()
    
    # Nascondi privacy label e bottoni azione se esistono
    if hasattr(root, 'privacy_label') and root.privacy_label and root.privacy_label.winfo_exists():
        root.privacy_label.pack_forget()
    if hasattr(root, 'btn_action_frame') and root.btn_action_frame and root.btn_action_frame.winfo_exists():
        root.btn_action_frame.pack_forget()
    
    status_label.config(text="Invio in corso...", fg="#FFFFFF", bg="#3498DB")
    root.update()  # Forza aggiornamento interfaccia

    msg = EmailMessage()
    msg["Subject"] = EMAIL_SUBJECT
    msg["From"] = SMTP_USER
    msg["To"] = email_to
    msg.set_content("Testo alternativo per client mail che non leggono HTML")
    
    # Crea versione HTML
    msg.add_alternative(EMAIL_HTML_BODY, subtype="html")
    
    # Aggiungi solo la foto come allegato scaricabile
    photo_filename = os.path.basename(current_photo_path)
    with open(current_photo_path, "rb") as f:
        photo_data = f.read()
        msg.add_attachment(photo_data, maintype="image", subtype="jpeg", 
                          filename=photo_filename)

    try:
        with smtplib.SMTP(SMTP_SERVER, SMTP_PORT) as server:
            server.starttls()
            server.login(SMTP_USER, SMTP_PASSWORD)
            server.send_message(msg)
        
        # Log invio riuscito
        log_email_send("OK", email_to, photo_filename)
        
        # Mostra schermata engagement dopo invio email
        show_engagement_screen()
        
    except Exception as e:
        # Log invio fallito
        log_email_send("KO", email_to, photo_filename)
        
        status_label.config(text=STATUS_EMAIL_ERROR.format(error=str(e)), fg="#FFFFFF", bg="#E74C3C")
        entry_email.config(highlightbackground="#FF6B6B", highlightcolor="#FF6B6B", highlightthickness=2)

# ================= GUI =================
root = tk.Tk()
root.configure(bg=UI_BG_COLOR)

# Fullscreen gestito diversamente per Windows e Linux
if system == "Windows":
    # Su Windows: usa solo fullscreen (overrideredirect causa conflitto)
    try:
        root.attributes("-fullscreen", True)
        print("🖥️ Windows: Fullscreen attivo")
    except:
        # Fallback: geometry manuale
        screen_width = root.winfo_screenwidth()
        screen_height = root.winfo_screenheight()
        root.geometry(f"{screen_width}x{screen_height}+0+0")
        root.state('zoomed')  # Massimizza finestra
        print(f"🖥️ Windows: Geometry {screen_width}x{screen_height}")
        
elif system == "Linux":
    # Su Linux/Raspberry: rimuovi barra titolo + geometry + cursore nascosto
    root.overrideredirect(True)
    try:
        # Ottieni dimensioni schermo
        screen_width = root.winfo_screenwidth()
        screen_height = root.winfo_screenheight()
        root.geometry(f"{screen_width}x{screen_height}+0+0")
        # Porta la finestra in primo piano
        root.lift()
        root.attributes('-topmost', True)
        root.after_idle(root.attributes, '-topmost', False)
        # Nascondi cursore per kiosk mode
        root.config(cursor="none")
        print(f"🖥️ Linux: Fullscreen {screen_width}x{screen_height}")
    except Exception as e:
        print(f"⚠️ Errore fullscreen Linux: {e}")
else:
    # macOS o altro
    try:
        root.attributes("-fullscreen", True)
        print(f"🖥️ {system}: Fullscreen attivo")
    except:
        pass

root.protocol("WM_DELETE_WINDOW", lambda: None)

# Crea canvas per sfondo con pattern logo
canvas_bg = tk.Canvas(root, bg=UI_BG_COLOR, highlightthickness=0)
canvas_bg.place(x=0, y=0, relwidth=1, relheight=1)

# Carica e crea pattern con logo (se disponibile)
def create_logo_pattern():
    """Crea pattern ripetuto con logo M9Lab in diagonale come watermark"""
    try:
        if os.path.exists(UI_LOGO_PATH):
            logo_pil = Image.open(UI_LOGO_PATH).convert("RGBA")
            
            # Ridimensiona logo per pattern
            logo_size = 180
            logo_pil = logo_pil.resize((logo_size, logo_size), Image.LANCZOS)
            
            # Applica opacità
            alpha = logo_pil.split()[3]
            alpha = alpha.point(lambda p: int(p * UI_LOGO_OPACITY))
            logo_pil.putalpha(alpha)
            
            # Ottieni dimensioni schermo
            screen_w = root.winfo_screenwidth()
            screen_h = root.winfo_screenheight()
            
            # Crea canvas pattern DIAGONALE
            spacing_x = 220  # Spaziatura orizzontale
            spacing_y = 220  # Spaziatura verticale
            diagonal_offset = spacing_x  # Offset per creare la diagonale
            
            # Inizializza lista per riferimenti immagini
            if not hasattr(canvas_bg, 'logo_images'):
                canvas_bg.logo_images = []
            
            # Crea pattern diagonale - partendo da coordinate negative per coprire tutto
            row = 0
            for y in range(-logo_size, screen_h + logo_size, spacing_y):
                # Calcola offset X per questa riga (effetto diagonale)
                x_offset = (row * diagonal_offset // 2) % (spacing_x * 2)
                
                for x in range(-logo_size + x_offset, screen_w + logo_size, spacing_x):
                    logo_tk = ImageTk.PhotoImage(logo_pil)
                    canvas_bg.create_image(x, y, image=logo_tk, anchor="nw")
                    canvas_bg.logo_images.append(logo_tk)
                
                row += 1
    except Exception as e:
        print(f"Logo pattern non disponibile: {e}")

# Applica pattern dopo che la finestra è visibile
root.after(100, create_logo_pattern)

# Frame per preview con cornice (IN ALTO)
preview_frame = tk.Frame(root, bg="#FFFFFF", bd=0, relief=tk.FLAT)
preview_frame.pack(expand=True, pady=PADDING_BUTTON, padx=PADDING_BUTTON)

# Cornice interna (bordo shadow)
preview_inner_frame = tk.Frame(preview_frame, bg="#E0E0E0", bd=8, relief=tk.FLAT)
preview_inner_frame.pack(padx=4, pady=4)

# Label preview dentro la cornice
preview_label = tk.Label(preview_inner_frame, bg="#000000", borderwidth=0)
preview_label.pack()

# Countdown - Disegnato direttamente sull'immagine preview (trasparente)
root.countdown_active = False
root.countdown_number = 0

# Frame per status label con sfondo (SOTTO LA PREVIEW)
status_frame = tk.Frame(root, bg="#2C3E50", relief=tk.FLAT, borderwidth=0)
status_frame.pack(pady=PADDING_FRAME)

# Status label - Testo adattivo da .env
status_label = tk.Label(status_frame, text=STATUS_READY, 
                        fg="#FFFFFF", bg="#2C3E50", 
                        font=("Arial", FONT_SIZE_STATUS, "bold"),
                        borderwidth=0,
                        padx=20, pady=8)
status_label.pack()

# Frame bottoni (IN BASSO)
btn_frame = tk.Frame(root, bg=UI_BG_COLOR)
btn_frame.pack(pady=PADDING_BUTTON)

# Bottone SCATTA FOTO - Design flat moderno con dimensioni da .env (ridotto 10px)
btn_shoot = tk.Button(btn_frame, text=BTN_SHOOT_TEXT, font=("Arial", FONT_SIZE_BUTTON_MAIN, "bold"), 
                      bg="#4CAF50", fg="white", 
                      activebackground="#45a049", activeforeground="white",
                      width=20, height=1,  # Ridotto da 2 a 1
                      pady=5,  # Aggiungo padding interno per compensare
                      relief=tk.FLAT, borderwidth=0,
                      cursor="hand2")
btn_shoot.grid(row=0, column=0, padx=10)

# Bottone ANNULLA - Design flat con dimensioni da .env (ridotto come gli altri)
btn_cancel = tk.Button(btn_frame, text=BTN_CANCEL_TEXT, font=("Arial", FONT_SIZE_BUTTON_SECONDARY), 
                        bg="#757575", fg="white", 
                        activebackground="#616161", activeforeground="white",
                        width=15, height=1,  # Ridotto da 2 a 1
                        pady=5,  # Aggiungo padding interno per compensare
                        command=reset_kiosk, 
                        relief=tk.FLAT, borderwidth=0,
                        cursor="hand2")

# ================= CONTROLLI DA TASTIERA =================
def on_key_press(event):
    """Gestisce i comandi da tastiera"""
    # ESC chiude sempre l'applicazione
    if event.keysym == 'Escape':
        if mode == MODE_RESULT or mode == "ENGAGEMENT":
            # Se in modalità risultato o engagement, torna all'inizio invece di chiudere
            reset_kiosk()
        else:
            # Altrimenti chiudi l'applicazione
            cap.release()
            root.destroy()
        return
    
    if mode == MODE_PREVIEW:
        # Modalità PREVIEW: SPAZIO o INVIO per scattare foto
        if event.keysym in ['space', 'Space', 'Return']:
            if btn_shoot.winfo_ismapped() and btn_shoot['state'] == 'normal':
                start_countdown()
    
    elif mode == MODE_RESULT:
        # Reset timer inattività ad ogni pressione tasto
        reset_inactivity_timer()
        
        # Modalità RISULTATO
        if event.keysym == 'Return':
            # INVIO: comportamento dipende dal focus
            focused_widget = root.focus_get()
            
            if focused_widget == entry_email:
                # Focus su email → invia email
                send_email()
            elif focused_widget == btn_cancel:
                # Focus su ANNULLA → torna all'inizio
                reset_kiosk()
            elif focused_widget == btn_send:
                # Focus su INVIA → invia email
                send_email()
    
    elif mode == "ENGAGEMENT":
        # Modalità ENGAGEMENT: SPAZIO o INVIO per tornare all'inizio
        if event.keysym in ['space', 'Space', 'Return']:
            reset_kiosk()

# Bind tasti globali
root.bind('<KeyPress>', on_key_press)

# Bind attività su campo email per reset timer
def on_email_activity(event):
    """Reset timer quando l'utente digita nel campo email"""
    reset_inactivity_timer()

# Campo email - Design flat con dimensioni da .env
entry_frame = tk.Frame(root, bg=UI_BG_COLOR)
entry_email = tk.Entry(entry_frame, font=("Arial", FONT_SIZE_EMAIL_ENTRY), width=28, 
                       relief=tk.FLAT, borderwidth=0,
                       bg="white", fg="#999999",  # Colore grigio per placeholder
                       insertbackground="#333333",
                       highlightthickness=3, 
                       highlightbackground="#E0E0E0", 
                       highlightcolor="#4CAF50")

# Funzioni per gestire placeholder
def add_placeholder(event=None):
    """Mostra il placeholder quando il campo è vuoto"""
    if entry_email.get() == "":
        entry_email.config(fg="#999999")
        entry_email.insert(0, EMAIL_PLACEHOLDER)

def remove_placeholder(event=None):
    """Rimuove il placeholder quando l'utente clicca o scrive"""
    if entry_email.get() == EMAIL_PLACEHOLDER:
        entry_email.delete(0, tk.END)
        entry_email.config(fg="#333333")

# Inizializza con placeholder
entry_email.insert(0, EMAIL_PLACEHOLDER)

# Bind eventi per placeholder
entry_email.bind('<FocusIn>', remove_placeholder)
entry_email.bind('<FocusOut>', add_placeholder)

# Bind eventi per reset timer
entry_email.bind('<KeyPress>', on_email_activity)
entry_email.bind('<KeyRelease>', on_email_activity)
entry_email.bind('<Button-1>', lambda e: reset_inactivity_timer())

# Bottone INVIA FOTO - Design flat con dimensioni da .env (ridotto 10px)
btn_send = tk.Button(root, text=BTN_SEND_TEXT, font=("Arial", FONT_SIZE_BUTTON_MAIN, "bold"), 
                     bg="#2196F3", fg="white", 
                     activebackground="#1976D2", activeforeground="white",
                     width=25, height=1,  # Ridotto da 2 a 1
                     pady=5,  # Aggiungo padding interno per compensare
                     command=send_email, 
                     relief=tk.FLAT, borderwidth=0,
                     cursor="hand2")

# Bind click su bottoni per reset timer
btn_send.bind('<Button-1>', lambda e: reset_inactivity_timer())
btn_cancel.bind('<Button-1>', lambda e: reset_inactivity_timer())

# Bottone ESCI - Design flat minimale
btn_exit = tk.Button(root, text=BTN_EXIT_TEXT, font=("Arial", 14, "bold"), 
                     bg="#E53935", fg="white", 
                     activebackground="#C62828", activeforeground="white",
                     command=lambda: (cap.release(), root.destroy()),
                     relief=tk.FLAT, borderwidth=0,
                     padx=20, pady=10,
                     cursor="hand2")
btn_exit.place(relx=0.98, rely=0.02, anchor="ne")

# Imposta ordine di navigazione TAB - ESCI deve essere ultimo
btn_shoot.configure(takefocus=True)
entry_email.configure(takefocus=True)
btn_send.configure(takefocus=True)
btn_cancel.configure(takefocus=True)
btn_exit.configure(takefocus=True)

# Forza l'ordine TAB corretto usando lift
btn_shoot.lift()
entry_email.lift()
btn_send.lift()
btn_cancel.lift()
btn_exit.lift()  # Ultimo

# Focus iniziale sul bottone scatta
root.after(200, lambda: btn_shoot.focus_set())

# ================= AVVIO =================
btn_shoot.config(command=start_countdown)
update_preview()
root.mainloop()
