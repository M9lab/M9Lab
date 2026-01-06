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

# ================= CARICAMENTO CONFIG DA .ENV =================
load_dotenv()

# Dimensioni (basate sullo sfondo 1366x1024 - formato 4:3)
PREVIEW_W = int(os.getenv("PREVIEW_W", 1366))
PREVIEW_H = int(os.getenv("PREVIEW_H", 1024))
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
    <p style='font-size: 16px; line-height: 1.6;'>Trovi in allegato la tua <strong>foto LEGO della Maker Faire 2026</strong>! 📸</p>
    <p style='font-size: 16px; line-height: 1.6;'>Condividila sui social con gli hashtag:<br/>
    <strong style='color: #3498DB;'>#MFTS26 #M9LAB #fotoricordo</strong></p>
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
BTN_SHOOT_TEXT = os.getenv("BTN_SHOOT_TEXT", "SCATTA FOTO LEGO")
BTN_CANCEL_TEXT = os.getenv("BTN_CANCEL_TEXT", "ANNULLA")
BTN_SEND_TEXT = os.getenv("BTN_SEND_TEXT", "INVIA FOTO")
BTN_EXIT_TEXT = os.getenv("BTN_EXIT_TEXT", "ESCI")

# Status messages
STATUS_READY = os.getenv("STATUS_READY", "Premi SCATTA FOTO LEGO o premi SPAZIO")
STATUS_ENTER_EMAIL = os.getenv("STATUS_ENTER_EMAIL", "Inserisci email e premi INVIO o TAB")
STATUS_CAMERA_ERROR = os.getenv("STATUS_CAMERA_ERROR", "Errore fotocamera")
STATUS_EMAIL_REQUIRED = os.getenv("STATUS_EMAIL_REQUIRED", "Inserisci un'email valida")
STATUS_EMAIL_INVALID = os.getenv("STATUS_EMAIL_INVALID", "Formato email non valido")
STATUS_EMAIL_SUCCESS = os.getenv("STATUS_EMAIL_SUCCESS", "Email inviata con successo!")
STATUS_EMAIL_ERROR = os.getenv("STATUS_EMAIL_ERROR", "Errore invio email: {error}")

# Entry placeholder
EMAIL_PLACEHOLDER = os.getenv("EMAIL_PLACEHOLDER", "Inserisci la tua email")

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
inactivity_timer = None  # Timer per timeout automatico

# ================= CREAZIONE CARTELLE =================
os.makedirs(INPUT_DIR, exist_ok=True)
os.makedirs(OUTPUT_DIR, exist_ok=True)

# ================= WEBCAM =================
cap = cv2.VideoCapture(0, cv2.CAP_DSHOW)
if not cap.isOpened():
    raise RuntimeError("Errore apertura fotocamera")

# Imposta risoluzione webcam a 16:9 direttamente (no crop necessario)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, WEBCAM_CAPTURE_W)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, WEBCAM_CAPTURE_H)

# Verifica risoluzione effettiva
actual_w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
actual_h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
webcam_ratio = actual_w / actual_h

print(f"Webcam configurata: {actual_w}x{actual_h} (ratio {webcam_ratio:.2f})")

# Calcola dimensioni preview 16:9 che stanno nello schermo
# Manteniamo il ratio 16:9 della webcam per mostrare esattamente quello che verrà fotografato
PREVIEW_RATIO = 16.0 / 9.0
if PREVIEW_W / PREVIEW_H > PREVIEW_RATIO:
    # Schermo più largo, limita per altezza
    PREVIEW_DISPLAY_H = PREVIEW_H
    PREVIEW_DISPLAY_W = int(PREVIEW_H * PREVIEW_RATIO)
else:
    # Schermo più alto, limita per larghezza
    PREVIEW_DISPLAY_W = PREVIEW_W
    PREVIEW_DISPLAY_H = int(PREVIEW_W / PREVIEW_RATIO)

print(f"Preview display: {PREVIEW_DISPLAY_W}x{PREVIEW_DISPLAY_H} (ratio {PREVIEW_DISPLAY_W/PREVIEW_DISPLAY_H:.2f})")
print(f"Canvas finale: {PHOTO_W}x{PHOTO_H}")

# ================= FUNZIONI =================
def resize_to_fit(image, target_size):
    target_w, target_h = target_size
    img_w, img_h = image.size
    target_ratio = target_w / target_h
    img_ratio = img_w / img_h

    if img_ratio > target_ratio:
        new_w = int(img_h * target_ratio)
        offset = (img_w - new_w) // 2
        image = image.crop((offset, 0, offset + new_w, img_h))
    else:
        new_h = int(img_w / target_ratio)
        offset = (img_h - new_h) // 2
        image = image.crop((0, offset, img_w, offset + new_h))

    return image.resize(target_size, Image.LANCZOS)

def update_preview():
    if mode == MODE_PREVIEW:
        ret, frame = cap.read()
        if ret:
            # Nessun crop - usa direttamente il frame 16:9 dalla webcam
            # Ridimensiona preview mantenendo 16:9
            frame_resized = cv2.resize(frame, (PREVIEW_DISPLAY_W, PREVIEW_DISPLAY_H), interpolation=cv2.INTER_LINEAR)
            img = Image.fromarray(cv2.cvtColor(frame_resized, cv2.COLOR_BGR2RGB))
            
            # Disegna countdown sull'immagine se attivo
            if hasattr(root, 'countdown_active') and root.countdown_active:
                draw = ImageDraw.Draw(img)
                text = str(root.countdown_number)
                # Font grande per countdown
                try:
                    font = ImageFont.truetype("arial.ttf", 200)
                except:
                    font = ImageFont.load_default()
                
                # Calcola posizione centrata
                bbox = draw.textbbox((0, 0), text, font=font)
                text_w = bbox[2] - bbox[0]
                text_h = bbox[3] - bbox[1]
                x = (PREVIEW_DISPLAY_W - text_w) // 2
                y = (PREVIEW_DISPLAY_H - text_h) // 2
                
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
        # EFFETTO FLASH BIANCO
        flash_effect()

def flash_effect():
    """Effetto flash bianco prima dello scatto"""
    # Crea overlay bianco a schermo intero
    flash_label = tk.Label(root, bg="white", borderwidth=0)
    flash_label.place(x=0, y=0, relwidth=1, relheight=1)
    
    # Fade out del flash (più veloce = più realistico)
    def fade_flash(alpha=1.0):
        if alpha > 0:
            # Calcola colore che va da bianco a trasparente
            gray_val = int(255 * alpha)
            color = f'#{gray_val:02x}{gray_val:02x}{gray_val:02x}'
            flash_label.config(bg=color)
            root.after(30, lambda: fade_flash(alpha - 0.2))
        else:
            flash_label.place_forget()
            flash_label.destroy()
            # Ora cattura la foto
            capture_photo()
    
    # Inizia il fade out dopo un breve momento
    root.after(50, lambda: fade_flash(1.0))

def capture_photo():
    global mode, current_photo_path
    ret, frame = cap.read()
    if not ret:
        status_label.config(text=STATUS_CAMERA_ERROR, fg="#FFFFFF", bg="#E74C3C")
        btn_shoot.config(state="normal")
        return

    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    input_path = os.path.join(INPUT_DIR, f"{ts}.png")
    output_path = os.path.join(OUTPUT_DIR, f"{ts}.jpg")

    # ================= SALVA FOTO ORIGINALE 16:9 (NO CROP) =================
    # La webcam è già configurata in 16:9, usiamo il frame completo
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
        font = ImageFont.truetype("arialbd.ttf", 60)  # Arial Bold più grande
    except:
        try:
            font = ImageFont.truetype("arial.ttf", 60)
        except:
            font = ImageFont.load_default()
    
    bbox = draw.textbbox((0,0), EVENT_TEXT, font=font)
    text_w = bbox[2] - bbox[0]
    text_h = bbox[3] - bbox[1]
    
    # Posiziona il testo in base alla dimensione dell'immagine finale
    final_w, final_h = final_image.size
    text_x = (final_w - text_w) // 2
    text_y = final_h - 120
    
    # Disegna rettangolo di sfondo semi-trasparente
    padding = 20
    bg_rect = [
        text_x - padding,
        text_y - padding,
        text_x + text_w + padding,
        text_y + text_h + padding
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
    
    # Disegna outline nero per maggior contrasto
    outline_width = 3
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
    
    # Mostra canvas finale 4:3 ridimensionato proporzionalmente per lo schermo
    # Calcola dimensioni mantenendo aspect ratio 4:3 del canvas
    canvas_ratio = image_pil.width / image_pil.height  # Dovrebbe essere 1600/1200 = 1.333 (4:3)
    
    if PREVIEW_W / PREVIEW_H > canvas_ratio:
        # Schermo più largo del canvas, limita per altezza
        display_h = PREVIEW_H
        display_w = int(PREVIEW_H * canvas_ratio)
    else:
        # Schermo più alto del canvas, limita per larghezza
        display_w = PREVIEW_W
        display_h = int(PREVIEW_W / canvas_ratio)
    
    imgtk = ImageTk.PhotoImage(image_pil.resize((display_w, display_h), Image.LANCZOS))
    preview_label.imgtk = imgtk
    preview_label.config(image=imgtk)
    
    status_label.config(text=STATUS_ENTER_EMAIL, fg="#FFFFFF", bg="#2C3E50")
    instructions_label.pack_forget()  # Nascondi istruzioni tastiera in modalità risultato
    btn_shoot.grid_forget()
    btn_cancel.grid(row=0, column=0)
    entry_email.delete(0, tk.END)  # Pulisce email precedente
    entry_frame.pack(pady=15)
    entry_email.pack(padx=80, pady=15, ipady=16)  # Maggior padding left/right
    btn_send.pack(pady=15)
    # Focus automatico sul campo email per iniziare a scrivere
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
    instructions_label.pack_forget()  # Nascondi istruzioni
    btn_frame.pack_forget()
    
    # Crea frame engagement
    engagement_frame = tk.Frame(root, bg=UI_BG_COLOR)
    engagement_frame.pack(expand=True, fill=tk.BOTH)
    
    # Titolo successo (frame per allineamento emoji)
    title_frame = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    title_frame.pack(pady=30)
    
    emoji_success = tk.Label(title_frame, text="✅", font=("Segoe UI Emoji", 36), 
                            fg="#FFFFFF", bg=UI_BG_COLOR)
    emoji_success.pack(side=tk.LEFT, padx=(0, 10))
    
    success_title = tk.Label(title_frame, 
                            text="Email inviata con successo!", 
                            font=("Arial", 36, "bold"), 
                            fg="#FFFFFF", bg=UI_BG_COLOR)
    success_title.pack(side=tk.LEFT)
    
    # Carica e mostra QR code Facebook
    qr_code_path = "m9lab_facebook_qrcode.png"
    if os.path.exists(qr_code_path):
        try:
            qr_img = Image.open(qr_code_path)
            qr_img = qr_img.resize((350, 350), Image.LANCZOS)
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
    
    emoji_phone = tk.Label(invite_frame, text="📱", font=("Segoe UI Emoji", 28), 
                          fg=UI_TEXT_COLOR, bg=UI_BG_COLOR)
    emoji_phone.pack(side=tk.LEFT, padx=(0, 10))
    
    invite_text = tk.Label(invite_frame, 
                           text="Scannerizza il QR code\no seguici sui social!", 
                           font=("Arial", 28, "bold"), 
                           fg=UI_TEXT_COLOR, bg=UI_BG_COLOR,
                           justify=tk.LEFT)
    invite_text.pack(side=tk.LEFT)
    
    # Link social (frame per allineamento emoji)
    social_frame_links = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    social_frame_links.pack(pady=15)
    
    emoji_fb = tk.Label(social_frame_links, text="👍", font=("Segoe UI Emoji", 24), 
                       fg="#FFD700", bg=UI_BG_COLOR)
    emoji_fb.pack(side=tk.LEFT, padx=(0, 5))
    
    fb_text = tk.Label(social_frame_links, text="Facebook: M9Lab", 
                      font=("Arial", 24), fg="#FFD700", bg=UI_BG_COLOR)
    fb_text.pack(side=tk.LEFT, padx=(0, 20))
    
    separator = tk.Label(social_frame_links, text="|", 
                        font=("Arial", 24), fg="#FFD700", bg=UI_BG_COLOR)
    separator.pack(side=tk.LEFT, padx=10)
    
    emoji_ig = tk.Label(social_frame_links, text="📸", font=("Segoe UI Emoji", 24), 
                       fg="#FFD700", bg=UI_BG_COLOR)
    emoji_ig.pack(side=tk.LEFT, padx=(0, 5))
    
    ig_text = tk.Label(social_frame_links, text="Instagram: @mezzaninelab", 
                      font=("Arial", 24), fg="#FFD700", bg=UI_BG_COLOR)
    ig_text.pack(side=tk.LEFT)
    
    # Messaggio grazie (frame per allineamento emoji)
    thanks_frame = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    thanks_frame.pack(pady=20)
    
    thanks_text = tk.Label(thanks_frame, 
                          text="Grazie per aver visitato il nostro stand!", 
                          font=("Arial", 20), 
                          fg=UI_TEXT_COLOR, bg=UI_BG_COLOR)
    thanks_text.pack(side=tk.LEFT, padx=(0, 10))
    
    emoji_rocket = tk.Label(thanks_frame, text="🚀", font=("Segoe UI Emoji", 20), 
                           fg=UI_TEXT_COLOR, bg=UI_BG_COLOR)
    emoji_rocket.pack(side=tk.LEFT)
    
    # Bottone per scattare un'altra foto (frame per allineamento emoji)
    btn_frame_another = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    btn_frame_another.pack(pady=30)
    
    btn_another = tk.Button(btn_frame_another, 
                            text="  SCATTA UN'ALTRA FOTO", 
                            font=("Arial", 24, "bold"), 
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
    
    # Mostra elementi standard
    status_frame.pack(pady=15)
    instructions_label.pack(pady=5)  # Mostra istruzioni tastiera
    preview_frame.pack(expand=True, pady=20, padx=20)
    btn_frame.pack(pady=20)
    
    entry_frame.pack_forget()
    entry_email.pack_forget()
    btn_send.pack_forget()
    btn_cancel.grid_forget()
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

def send_email():
    email_to = entry_email.get().strip()
    
    # Controlla se l'email è vuota
    if not email_to:
        status_label.config(text=STATUS_EMAIL_REQUIRED, fg="#FFFFFF", bg="#E74C3C")
        entry_email.config(highlightbackground="#FF6B6B", highlightcolor="#FF6B6B", highlightthickness=2)
        return
    
    # Valida il formato dell'email
    if not validate_email(email_to):
        status_label.config(text=STATUS_EMAIL_INVALID, fg="#FFFFFF", bg="#E74C3C")
        entry_email.config(highlightbackground="#FF6B6B", highlightcolor="#FF6B6B", highlightthickness=2)
        return
    
    # Email valida - Nascondi immediatamente il campo email e i bottoni
    entry_frame.pack_forget()
    entry_email.pack_forget()
    btn_send.pack_forget()
    btn_cancel.grid_forget()
    
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
    with open(current_photo_path, "rb") as f:
        photo_data = f.read()
        msg.add_attachment(photo_data, maintype="image", subtype="jpeg", 
                          filename=os.path.basename(current_photo_path))

    try:
        with smtplib.SMTP(SMTP_SERVER, SMTP_PORT) as server:
            server.starttls()
            server.login(SMTP_USER, SMTP_PASSWORD)
            server.send_message(msg)
        
        # Mostra schermata engagement dopo invio email
        show_engagement_screen()
        
    except Exception as e:
        status_label.config(text=STATUS_EMAIL_ERROR.format(error=str(e)), fg="#FFFFFF", bg="#E74C3C")
        entry_email.config(highlightbackground="#FF6B6B", highlightcolor="#FF6B6B", highlightthickness=2)

# ================= GUI =================
root = tk.Tk()
root.attributes("-fullscreen", True)
root.configure(bg=UI_BG_COLOR)
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

# Frame per preview con cornice
preview_frame = tk.Frame(root, bg="#FFFFFF", bd=0, relief=tk.FLAT)
preview_frame.pack(expand=True, pady=20, padx=20)

# Cornice interna (bordo shadow)
preview_inner_frame = tk.Frame(preview_frame, bg="#E0E0E0", bd=8, relief=tk.FLAT)
preview_inner_frame.pack(padx=4, pady=4)

# Label preview dentro la cornice
preview_label = tk.Label(preview_inner_frame, bg="#000000", borderwidth=0)
preview_label.pack()

# Countdown - Disegnato direttamente sull'immagine preview (trasparente)
root.countdown_active = False
root.countdown_number = 0

# Frame per status label con sfondo per leggibilità
status_frame = tk.Frame(root, bg="#2C3E50", relief=tk.FLAT, borderwidth=0)
status_frame.pack(pady=15)

# Status label - Testo più grande e bold con sfondo scuro
status_label = tk.Label(status_frame, text=STATUS_READY, 
                        fg="#FFFFFF", bg="#2C3E50", 
                        font=("Arial", 24, "bold"),
                        borderwidth=0,
                        padx=30, pady=12)
status_label.pack()

# Label istruzioni tastiera - sotto lo status
instructions_label = tk.Label(root, 
                             text="⌨️ Premi SPAZIO o INVIO per scattare  •  ESC per uscire", 
                             fg=UI_ACCENT_COLOR, bg=UI_BG_COLOR,
                             font=("Arial", 16, "italic"),
                             borderwidth=0,
                             padx=20, pady=8)
instructions_label.pack(pady=5)

btn_frame = tk.Frame(root, bg=UI_BG_COLOR)
btn_frame.pack(pady=20)

# Bottone SCATTA FOTO - Design flat moderno
btn_shoot = tk.Button(btn_frame, text=BTN_SHOOT_TEXT, font=("Arial", 26, "bold"), 
                      bg="#4CAF50", fg="white", 
                      activebackground="#45a049", activeforeground="white",
                      width=20, height=2,
                      relief=tk.FLAT, borderwidth=0,
                      cursor="hand2",
                      command=lambda: start_countdown())
btn_shoot.grid(row=0, column=0, padx=10)

# Effetti hover per bottone SCATTA
def on_shoot_enter(e):
    if btn_shoot['state'] == 'normal':
        btn_shoot['bg'] = "#45a049"  # Verde più scuro

def on_shoot_leave(e):
    if btn_shoot['state'] == 'normal':
        btn_shoot['bg'] = "#4CAF50"  # Verde originale

btn_shoot.bind("<Enter>", on_shoot_enter)
btn_shoot.bind("<Leave>", on_shoot_leave)

# Bottone ANNULLA - Design flat
btn_cancel = tk.Button(btn_frame, text=BTN_CANCEL_TEXT, font=("Arial", 20), 
                        bg="#757575", fg="white", 
                        activebackground="#616161", activeforeground="white",
                        width=15, height=2, 
                        command=reset_kiosk, 
                        relief=tk.FLAT, borderwidth=0,
                        cursor="hand2")

# Effetti hover per bottone ANNULLA
def on_cancel_enter(e):
    btn_cancel['bg'] = "#616161"  # Grigio più scuro

def on_cancel_leave(e):
    btn_cancel['bg'] = "#757575"  # Grigio originale

btn_cancel.bind("<Enter>", on_cancel_enter)
btn_cancel.bind("<Leave>", on_cancel_leave)

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

# Campo email - Design flat con padding interno
entry_frame = tk.Frame(root, bg=UI_BG_COLOR)
entry_email = tk.Entry(entry_frame, font=("Arial", 24), width=28, 
                       relief=tk.FLAT, borderwidth=0,
                       bg="white", fg="#333333",
                       insertbackground="#333333",
                       highlightthickness=3, 
                       highlightbackground="#E0E0E0", 
                       highlightcolor="#4CAF50")

# Bind eventi per reset timer
entry_email.bind('<KeyPress>', on_email_activity)
entry_email.bind('<KeyRelease>', on_email_activity)
entry_email.bind('<Button-1>', lambda e: reset_inactivity_timer())

# Bottone INVIA FOTO - Design flat
btn_send = tk.Button(root, text=BTN_SEND_TEXT, font=("Arial", 24, "bold"), 
                     bg="#2196F3", fg="white", 
                     activebackground="#1976D2", activeforeground="white",
                     width=25, height=2, 
                     command=send_email, 
                     relief=tk.FLAT, borderwidth=0,
                     cursor="hand2")

# Effetti hover per bottone INVIA
def on_send_enter(e):
    if btn_send['state'] == 'normal':
        btn_send['bg'] = "#1976D2"  # Blu più scuro

def on_send_leave(e):
    if btn_send['state'] == 'normal':
        btn_send['bg'] = "#2196F3"  # Blu originale

btn_send.bind("<Enter>", on_send_enter)
btn_send.bind("<Leave>", on_send_leave)

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

# Effetti hover per bottone EXIT
def on_exit_enter(e):
    btn_exit['bg'] = "#C62828"  # Rosso più scuro

def on_exit_leave(e):
    btn_exit['bg'] = "#E53935"  # Rosso originale

btn_exit.bind("<Enter>", on_exit_enter)
btn_exit.bind("<Leave>", on_exit_leave)

# Imposta ordine di navigazione TAB
btn_shoot.configure(takefocus=True)
btn_cancel.configure(takefocus=True)
entry_email.configure(takefocus=True)
btn_send.configure(takefocus=True)
btn_exit.configure(takefocus=True)

# Focus iniziale sul bottone scatta
root.after(200, lambda: btn_shoot.focus_set())

# ================= AVVIO =================
# Comando già assegnato al bottone nella creazione
update_preview()
root.mainloop()
