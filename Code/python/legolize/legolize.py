# -*- coding: utf-8 -*-
"""
Legolize: cattura foto dalla webcam (1024x683, 3:2), invia a Replicate per legolizzare,
mostra risultato e invio email. Stesso flusso e grafica di fotomachine, senza fotomontaggio.
"""
import tkinter as tk
import cv2  # type: ignore[import-untyped]
from PIL import Image, ImageTk, ImageDraw, ImageFont  # type: ignore[import-untyped]
import os
import urllib.request
from datetime import datetime
import smtplib
from email.message import EmailMessage
from dotenv import load_dotenv  # type: ignore[import-untyped]
import re
import platform
import threading
import traceback
import sys

# Carica .env: TEMPORANEO per test - se esiste .env.raspberry lo usiamo (simula Raspberry su Windows)
# Dopo i test: rimettere condizione solo Linux per .env.raspberry
_script_dir_env = os.path.dirname(os.path.abspath(__file__))
_env_rasp = os.path.join(_script_dir_env, ".env.raspberry")
if os.path.exists(_env_rasp):
    load_dotenv(_env_rasp)
    print("Caricato: .env.raspberry (test simulazione Raspberry 1360x768)")
else:
    _env_path = os.path.join(_script_dir_env, ".env")
    if not os.path.exists(_env_path):
        _env_path = os.path.join(os.path.dirname(_script_dir_env), ".env")
    load_dotenv(_env_path)
    print("Caricato: .env")

system = platform.system()

# ================= DIMENSIONI: width 1024, aspect ratio 3:2 =================
# Preview LIVE e RESULT: 3:2
PHOTO_W = int(os.getenv("PHOTO_W", 1024))
PHOTO_H = int(os.getenv("PHOTO_H", 683))   # 1024 * 2/3
PREVIEW_LIVE_W = int(os.getenv("PREVIEW_LIVE_W", 1024))
PREVIEW_LIVE_H = int(os.getenv("PREVIEW_LIVE_H", 683))
PREVIEW_RESULT_W = int(os.getenv("PREVIEW_RESULT_W", 1024))
PREVIEW_RESULT_H = int(os.getenv("PREVIEW_RESULT_H", 683))

WEBCAM_CAPTURE_W = int(os.getenv("WEBCAM_CAPTURE_W", 1024))
WEBCAM_CAPTURE_H = int(os.getenv("WEBCAM_CAPTURE_H", 683))
COUNTDOWN_SECONDS = int(os.getenv("COUNTDOWN_SECONDS", 3))

INPUT_DIR = os.getenv("INPUT_DIR", "input_photos")
OUTPUT_DIR = os.getenv("OUTPUT_DIR", "output_photos")

# Replicate - nome modello in formato owner/name o owner/name:version (es. openai/gpt-image-1.5)
REPLICATE_TOKEN = os.getenv("replicateToken") or os.getenv("REPLICATE_API_TOKEN")
_model_ref = os.getenv("REPLICATE_MODEL") or os.getenv("modelID") or os.getenv("REPLICATE_MODEL_ID") or ""
# Se e' solo un hash (senza /) non e' valido, usa default
REPLICATE_MODEL = _model_ref if "/" in _model_ref else "openai/gpt-image-1.5"
# Colori logo per il prompt (render LEGO)
LOGO_TEXT_COLOR = os.getenv("LOGO_TEXT_COLOR", "F5EA25")
LOGO_BG_COLOR = os.getenv("LOGO_BG_COLOR", "5BA4D1")


def _build_replicate_prompt():
    base = (
        "Turn the person's head into an official LEGO minifigure with authentic hair.\n"
        "The face must be in authentic LEGO minifigure style: no nose (omit/remove the nose, smooth yellow head like real LEGO).\n"
        "Place the minifigure beside LEGO railway tracks in a simple railway diorama.\n"
        "Visible studs, correct proportions.\n"
        f"Include the M9Lab logo (yellow text #{LOGO_TEXT_COLOR} on light blue #{LOGO_BG_COLOR}).\n"
        "Macro photo style, natural lighting.\n"
        "No cartoon or digital art style."
    )
    return os.getenv("REPLICATE_LEGO_PROMPT", base)
    
#
#def _build_replicate_prompt():
#    base = (
#        "Turn the person's head into an official LEGO minifigure with authentic LEGO hair.\n"
#        f"Include the M9Lab / MezzanineLab logo (yellow text #{LOGO_TEXT_COLOR} on light blue #{LOGO_BG_COLOR}).\n"
#        "Place the minifigure in a simple LEGO railway setting.\n"
#        "Macro photo style, natural lighting.\n"
#        "No cartoon or digital art style."
#    )
#    return os.getenv("REPLICATE_LEGO_PROMPT", base)    

REPLICATE_LEGO_PROMPT = _build_replicate_prompt()
REPLICATE_QUALITY = os.getenv("REPLICATE_QUALITY", "medium")
REPLICATE_OUTPUT_FORMAT = os.getenv("REPLICATE_OUTPUT_FORMAT", "webp")

# Email
SMTP_SERVER = os.getenv("SMTP_SERVER", "smtp.gmail.com")
SMTP_PORT = int(os.getenv("SMTP_PORT", 587))
SMTP_USER = os.getenv("SMTP_USER", "")
SMTP_PASSWORD = os.getenv("SMTP_PASSWORD", "")
EMAIL_SUBJECT = os.getenv("EMAIL_SUBJECT", "La tua foto LEGO").replace('\n', ' ').replace('\r', ' ').strip()

# Email body come da fotomachine (schermata invio / contenuto mail)
EMAIL_HTML_BODY_DEFAULT = """<html>
<body style='font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto; padding: 20px;'>
    <h2 style='color: #2C3E50; text-align: center;'>Grazie per aver visitato il nostro stand!</h2>
    <p style='font-size: 16px; line-height: 1.6;'>Trovi in allegato la tua <strong>foto della Maker Faire 2026</strong>!</p>
    <p style='font-size: 16px; line-height: 1.6;'>Condividila sui social con gli hashtag:<br/>
    <strong style='color: #3498DB;'> #m9lab #mfts2026 #fotoricordolego #legofotomachine #maker</strong></p>
    <hr style='border: none; border-top: 2px solid #ddd; margin: 30px 0;'/>
    <h3 style='color: #3498DB; text-align: center;'>Seguici sui nostri social!</h3>
    <p style='font-size: 15px; text-align: center; color: #555;'>
        Se ti e' piaciuto il nostro stand allora <strong>lasciaci un LIKE</strong> sulle nostre pagine social!
    </p>
    <table style='width: 100%; margin: 20px 0;'>
        <tr>
            <td style='text-align: center; padding: 20px;'>
                <a href='https://www.facebook.com/m9lab/'
                   style='display: inline-block; margin: 15px 10px; background-color: #1877F2; color: white;
                          padding: 14px 28px; text-decoration: none; border-radius: 8px; font-weight: bold;
                          font-size: 15px; box-shadow: 0 3px 6px rgba(24,119,242,0.3);'>
                    Facebook: M9Lab
                </a>
                <a href='https://www.instagram.com/mezzaninelab'
                   style='display: inline-block; margin: 15px 10px; background-color: #E4405F; color: white;
                          padding: 14px 28px; text-decoration: none; border-radius: 8px; font-weight: bold;
                          font-size: 15px; box-shadow: 0 3px 6px rgba(228,64,95,0.3);'>
                    Instagram: @mezzaninelab
                </a>
            </td>
        </tr>
    </table>
    <div style='background-color: #f8f9fa; padding: 20px; border-radius: 10px; text-align: center; margin-top: 30px;'>
        <p style='color: #2C3E50; font-size: 14px; margin: 0;'>Grazie per il tuo supporto!</p>
        <p style='color: #7f8c8d; font-size: 13px; margin: 10px 0 0 0;'>
            <strong>MezzanineLab</strong><br/>Lego Trains & Arduino Sketches
        </p>
    </div>
</body>
</html>"""
EMAIL_HTML_BODY = os.getenv("EMAIL_HTML_BODY", EMAIL_HTML_BODY_DEFAULT)

# UI testi
BTN_SHOOT_TEXT = os.getenv("BTN_SHOOT_TEXT", "SCATTA UNA FOTO")
BTN_CANCEL_TEXT = os.getenv("BTN_CANCEL_TEXT", "ANNULLA")
BTN_SEND_TEXT = os.getenv("BTN_SEND_TEXT", "INVIA FOTO")
BTN_EXIT_TEXT = os.getenv("BTN_EXIT_TEXT", "ESCI")
STATUS_READY = os.getenv("STATUS_READY", "Premi SCATTA UNA FOTO (o premi SPAZIO o INVIO)")
STATUS_ENTER_EMAIL = os.getenv("STATUS_ENTER_EMAIL", "Inserisci email e premi INVIO o TAB")
STATUS_CAMERA_ERROR = os.getenv("STATUS_CAMERA_ERROR", "Errore fotocamera")
STATUS_LEGO_PROCESSING = os.getenv("STATUS_LEGO_PROCESSING", "Elaborazione Foto in corso...")
STATUS_LEGO_ERROR = os.getenv("STATUS_LEGO_ERROR", "Errore elaborazione: {error}")
STATUS_EMAIL_REQUIRED = os.getenv("STATUS_EMAIL_REQUIRED", "Inserisci un'email valida")
STATUS_EMAIL_INVALID = os.getenv("STATUS_EMAIL_INVALID", "Formato email non valido")
STATUS_EMAIL_SUCCESS = os.getenv("STATUS_EMAIL_SUCCESS", "Email inviata con successo!")
STATUS_EMAIL_ERROR = os.getenv("STATUS_EMAIL_ERROR", "Errore invio email: {error}")
STATUS_CAPTURE_PREVIEW = os.getenv("STATUS_CAPTURE_PREVIEW", "Clicca SPAZIO o INVIO per avviare legolizzazione, o ESC per annullare")

BTN_LEGOLIZE_TEXT = os.getenv("BTN_LEGOLIZE_TEXT", "LEGOLIZZA")
EMAIL_PLACEHOLDER = os.getenv("EMAIL_PLACEHOLDER", "Inserisci la tua email")
EMAIL_PRIVACY_NOTICE = os.getenv("EMAIL_PRIVACY_NOTICE", "Useremo la tua email solo per questa operazione. Niente archiviazione, niente spam.")
EVENT_TEXT = os.getenv("EVENT_TEXT", "Benvenuti al LEGO Museum! #LEGOTrains")

# Colori UI
UI_BG_COLOR = os.getenv("UI_BG_COLOR", "#5DADE2")
UI_TEXT_COLOR = os.getenv("UI_TEXT_COLOR", "#FFFFFF")
UI_ACCENT_COLOR = os.getenv("UI_ACCENT_COLOR", "#FFD700")
# Cartella fotomachine per logo, puntamento e asset condivisi
_script_dir = os.path.dirname(os.path.abspath(__file__))
# File per persistere le chiamate Replicate rimanenti (tra un riavvio e l'altro)
_REPLICATE_CALLS_FILE = os.path.join(_script_dir, ".replicate_calls_remaining")
REPLICATE_CALLS_LEFT_INIT = int(os.getenv("REPLICATE_CALLS_LEFT", "150"))


def _get_replicate_remaining():
    """Legge chiamate rimanenti da file; se assente usa REPLICATE_CALLS_LEFT da env."""
    try:
        with open(_REPLICATE_CALLS_FILE, "r", encoding="utf-8") as f:
            return int(f.read().strip())
    except (FileNotFoundError, ValueError):
        return REPLICATE_CALLS_LEFT_INIT


def _decrement_replicate_remaining():
    """Decrementa di 1, salva su file, ritorna il nuovo valore."""
    n = _get_replicate_remaining()
    if n > 0:
        n -= 1
    try:
        with open(_REPLICATE_CALLS_FILE, "w", encoding="utf-8") as f:
            f.write(str(n))
    except Exception:
        pass
    return n


FOTOMACHINE_DIR = os.path.normpath(os.path.join(_script_dir, "..", "fotomachine"))
# Logo: prima in fotomachine, poi legolize, poi env
_logo_env = os.getenv("UI_LOGO_PATH", "logo.png")
if os.path.isabs(_logo_env):
    UI_LOGO_PATH = _logo_env
else:
    _candidates = [
        os.path.join(FOTOMACHINE_DIR, _logo_env),
        os.path.join(_script_dir, _logo_env),
        os.path.join(os.path.dirname(_script_dir), _logo_env),
    ]
    UI_LOGO_PATH = next((p for p in _candidates if os.path.exists(p)), os.path.join(FOTOMACHINE_DIR, _logo_env))
# Puntamento (es. crosshair/target): da fotomachine
PUNTAMENTO_PATH = os.path.join(FOTOMACHINE_DIR, os.getenv("PUNTAMENTO_FILE", "puntamento.png"))
if not os.path.exists(PUNTAMENTO_PATH):
    PUNTAMENTO_PATH = None
# QR code e asset schermata finale da fotomachine
QR_CODE_PATH = os.path.join(FOTOMACHINE_DIR, "m9lab_facebook_qrcode.png") if os.path.exists(os.path.join(FOTOMACHINE_DIR, "m9lab_facebook_qrcode.png")) else "m9lab_facebook_qrcode.png"
UI_LOGO_OPACITY = float(os.getenv("UI_LOGO_OPACITY", 0.1))
# Puntamento (crosshair/target) caricato da fotomachine, usato in preview
_puntamento_pil = None
if PUNTAMENTO_PATH and os.path.exists(PUNTAMENTO_PATH):
    try:
        _puntamento_pil = Image.open(PUNTAMENTO_PATH).convert("RGBA")
    except Exception:
        _puntamento_pil = None

# Globali
MODE_PREVIEW = 0
MODE_RESULT = 1
MODE_CAPTURE_PREVIEW = 2  # Anteprima foto scattata: Annulla o Legolizza
mode = MODE_PREVIEW
current_photo_path = None
captured_frame = None
inactivity_timer = None

os.makedirs(INPUT_DIR, exist_ok=True)
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Font e padding
FONT_SIZE_COUNTDOWN = int(os.getenv("FONT_SIZE_COUNTDOWN", 200))
FONT_SIZE_STATUS = int(os.getenv("FONT_SIZE_STATUS", 24))
FONT_SIZE_BUTTON_MAIN = int(os.getenv("FONT_SIZE_BUTTON_MAIN", 26))
FONT_SIZE_BUTTON_SECONDARY = int(os.getenv("FONT_SIZE_BUTTON_SECONDARY", 20))
FONT_SIZE_EMAIL_ENTRY = int(os.getenv("FONT_SIZE_EMAIL_ENTRY", 24))
PADDING_FRAME = int(os.getenv("PADDING_FRAME", 20))
PADDING_BUTTON = int(os.getenv("PADDING_BUTTON", 20))
QR_CODE_SIZE = int(os.getenv("QR_CODE_SIZE", 350))
FONT_SIZE_ENGAGEMENT_TITLE = int(os.getenv("FONT_SIZE_ENGAGEMENT_TITLE", 36))
FONT_SIZE_ENGAGEMENT_TEXT = int(os.getenv("FONT_SIZE_ENGAGEMENT_TEXT", 28))
FONT_SIZE_ENGAGEMENT_SOCIAL = int(os.getenv("FONT_SIZE_ENGAGEMENT_SOCIAL", 24))
FONT_SIZE_ENGAGEMENT_THANKS = int(os.getenv("FONT_SIZE_ENGAGEMENT_THANKS", 20))
FONT_SIZE_EVENT_TEXT = int(os.getenv("FONT_SIZE_EVENT_TEXT", 60))

if system == "Linux":
    EMOJI_FONT = "Noto Color Emoji"
else:
    EMOJI_FONT = "Segoe UI Emoji"

# Webcam
if system == "Windows":
    cap = cv2.VideoCapture(0, cv2.CAP_DSHOW)
elif system == "Linux":
    cap = cv2.VideoCapture(0, cv2.CAP_V4L2)
else:
    cap = cv2.VideoCapture(0)

if not cap.isOpened():
    raise RuntimeError("Errore apertura fotocamera.")

cap.set(cv2.CAP_PROP_FRAME_WIDTH, WEBCAM_CAPTURE_W)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, WEBCAM_CAPTURE_H)
actual_w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
actual_h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
print(f"Webcam: {actual_w}x{actual_h} (3:2)")
print(f"Output: {PHOTO_W}x{PHOTO_H} (3:2)")


def call_replicate_legolize(input_path, on_success, on_error):
    """Chiama Replicate per legolizzare l'immagine. Esegue in thread, callback sulla main thread."""
    if not REPLICATE_TOKEN:
        on_error("replicateToken deve essere impostato in .env")
        return
    if not REPLICATE_MODEL:
        on_error("REPLICATE_MODEL (nome modello, es. openai/gpt-image-1.5) deve essere in .env")
        return
    os.environ["REPLICATE_API_TOKEN"] = REPLICATE_TOKEN

    def run():
        try:
            import replicate  # type: ignore[import-untyped]
            print(f"[Replicate] Modello: {REPLICATE_MODEL}", flush=True)
            # Il client Replicate richiede file aperto per upload; teniamo aperto per tutta la run
            f = open(input_path, "rb")
            try:
                output = replicate.run(
                    REPLICATE_MODEL,
                    input={
                        "prompt": REPLICATE_LEGO_PROMPT,
                        "quality": REPLICATE_QUALITY,
                        "background": "auto",
                        "moderation": "auto",
                        "aspect_ratio": "3:2",
                        "input_images": [f],
                        "output_format": REPLICATE_OUTPUT_FORMAT,
                        "input_fidelity": "low",
                        "number_of_images": 1,
                        "output_compression": 90,
                    },
                )
            finally:
                f.close()
            if output and len(output) > 0:
                url = output[0]
                if isinstance(url, str):
                    on_success(url)
                else:
                    on_success(str(url))
            else:
                on_error("Nessuna immagine restituita da Replicate")
        except Exception as e:
            # Debug: stampa traceback completo in console
            print("[Replicate] ERRORE:", file=sys.stderr, flush=True)
            traceback.print_exc(file=sys.stderr)
            on_error(str(e))

    t = threading.Thread(target=run, daemon=True)
    t.start()


def download_image(url, save_path):
    """Scarica immagine da URL e salva in save_path."""
    req = urllib.request.Request(url, headers={"User-Agent": "Legolize/1.0"})
    with urllib.request.urlopen(req) as resp:
        data = resp.read()
    with open(save_path, "wb") as f:
        f.write(data)
    return save_path


_event_text_font_cache = None


def load_font_cached():
    """Font per EVENT_TEXT sulla foto (come fotomachine)."""
    global _event_text_font_cache
    if _event_text_font_cache is None:
        try:
            _event_text_font_cache = ImageFont.truetype("arialbd.ttf", FONT_SIZE_EVENT_TEXT)
        except Exception:
            try:
                _event_text_font_cache = ImageFont.truetype("arial.ttf", FONT_SIZE_EVENT_TEXT)
            except Exception:
                try:
                    _event_text_font_cache = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", FONT_SIZE_EVENT_TEXT)
                except Exception:
                    _event_text_font_cache = ImageFont.load_default()
    return _event_text_font_cache


def apply_event_text_to_image(img):
    """Disegna EVENT_TEXT sulla foto come in fotomachine: sfondo nero semi-trasparente, outline nero, testo giallo."""
    draw = ImageDraw.Draw(img)
    font = load_font_cached()
    bbox = draw.textbbox((0, 0), EVENT_TEXT, font=font)
    text_w = bbox[2] - bbox[0]
    text_h = bbox[3] - bbox[1]
    final_w, final_h = img.size
    text_x = (final_w - text_w) // 2
    text_y = final_h - 120
    outline_width = 3
    padding_h = 25
    padding_v = 20
    bg_rect = [
        text_x - padding_h - outline_width,
        text_y - padding_v - outline_width + bbox[1],
        text_x + text_w + padding_h + outline_width,
        text_y + text_h + padding_v + outline_width + bbox[1],
    ]
    overlay = Image.new("RGBA", img.size, (0, 0, 0, 0))
    overlay_draw = ImageDraw.Draw(overlay)
    overlay_draw.rectangle(bg_rect, fill=(0, 0, 0, 180))
    if img.mode != "RGBA":
        img = img.convert("RGBA")
    img = Image.alpha_composite(img, overlay)
    draw = ImageDraw.Draw(img)
    for adj_x in range(-outline_width, outline_width + 1):
        for adj_y in range(-outline_width, outline_width + 1):
            if adj_x != 0 or adj_y != 0:
                draw.text((text_x + adj_x, text_y + adj_y), EVENT_TEXT, fill="black", font=font)
    draw.text((text_x, text_y), EVENT_TEXT, fill="#FFD700", font=font)
    return img.convert("RGB")


# Overlay caricamento a schermo intero durante elaborazione Replicate
_loading_overlay = None
_loading_spinner_angle = [0]  # lista per poterla modificare nel closure

def _spinner_step():
    """Anima l'arco del cerchio di caricamento."""
    if _loading_overlay is None or not _loading_overlay.winfo_exists():
        return
    try:
        canvas = getattr(_loading_overlay, "spinner_canvas", None)
        if canvas is None:
            return
        _loading_spinner_angle[0] = (_loading_spinner_angle[0] + 12) % 360
        canvas.delete("spinner")
        cx, cy, r = 80, 80, 50
        canvas.create_arc(
            cx - r, cy - r, cx + r, cy + r,
            start=_loading_spinner_angle[0], extent=100,
            style=tk.ARC, outline=UI_ACCENT_COLOR, width=6, tags="spinner"
        )
        _loading_overlay.after_id = _loading_overlay.after(50, _spinner_step)
    except Exception:
        pass


def show_loading_overlay():
    """Mostra overlay a schermo intero con rotellina durante elaborazione."""
    global _loading_overlay
    hide_loading_overlay()
    _loading_spinner_angle[0] = 0
    overlay = tk.Frame(root, bg=UI_BG_COLOR)
    overlay.place(x=0, y=0, relwidth=1, relheight=1)
    overlay.lift()
    # Testo
    lbl = tk.Label(overlay, text=STATUS_LEGO_PROCESSING, font=("Arial", 28, "bold"),
                   fg=UI_TEXT_COLOR, bg=UI_BG_COLOR)
    lbl.place(relx=0.5, rely=0.45, anchor=tk.CENTER)
    # Canvas per rotellina (cerchio con arco rotante); su Raspberry 40px più in basso per non coprire il testo
    canvas = tk.Canvas(overlay, width=160, height=160, bg=UI_BG_COLOR, highlightthickness=0)
    if system == "Linux":
        canvas.place(relx=0.5, rely=0.55, y=40, anchor=tk.CENTER)
    else:
        canvas.place(relx=0.5, rely=0.55, anchor=tk.CENTER)
    overlay.spinner_canvas = canvas
    _loading_overlay = overlay
    _spinner_step()


def hide_loading_overlay():
    """Rimuove l'overlay di caricamento."""
    global _loading_overlay
    if _loading_overlay is not None:
        if hasattr(_loading_overlay, 'after_id') and _loading_overlay.after_id:
            try:
                _loading_overlay.after_cancel(_loading_overlay.after_id)
            except Exception:
                pass
        try:
            _loading_overlay.destroy()
        except Exception:
            pass
        _loading_overlay = None


def update_preview():
    if mode == MODE_PREVIEW:
        ret, frame = cap.read()
        if ret:
            frame_resized = cv2.resize(frame, (PREVIEW_LIVE_W, PREVIEW_LIVE_H), interpolation=cv2.INTER_LINEAR)
            img = Image.fromarray(cv2.cvtColor(frame_resized, cv2.COLOR_BGR2RGB))
            if _puntamento_pil is not None:
                pw, ph = _puntamento_pil.size
                x_pt = (PREVIEW_LIVE_W - pw) // 2
                y_pt = (PREVIEW_LIVE_H - ph) // 2
                img = img.convert("RGBA")
                img.paste(_puntamento_pil, (x_pt, y_pt), _puntamento_pil)
                img = img.convert("RGB")
            if hasattr(root, 'countdown_active') and root.countdown_active:
                draw = ImageDraw.Draw(img)
                text = str(root.countdown_number)
                try:
                    font = ImageFont.truetype("arial.ttf", FONT_SIZE_COUNTDOWN)
                except Exception:
                    try:
                        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", FONT_SIZE_COUNTDOWN)
                    except Exception:
                        font = ImageFont.load_default()
                bbox = draw.textbbox((0, 0), text, font=font)
                text_w, text_h = bbox[2] - bbox[0], bbox[3] - bbox[1]
                x = (PREVIEW_LIVE_W - text_w) // 2
                y = (PREVIEW_LIVE_H - text_h) // 2
                outline_width = 8
                for adj_x in range(-outline_width, outline_width + 1, 4):
                    for adj_y in range(-outline_width, outline_width + 1, 4):
                        draw.text((x + adj_x, y + adj_y), text, font=font, fill="black")
                draw.text((x, y), text, font=font, fill=UI_ACCENT_COLOR)
            imgtk = ImageTk.PhotoImage(img)
            preview_label.imgtk = imgtk
            preview_label.config(image=imgtk)
    if root.winfo_exists():
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
        capture_and_flash()


def capture_and_flash():
    global captured_frame
    for _ in range(3):
        cap.read()
    ret, captured_frame = cap.read()
    if not ret:
        status_label.config(text=STATUS_CAMERA_ERROR, fg="#FFFFFF", bg="#E74C3C")
        btn_shoot.config(state="normal")
        return
    flash_label = tk.Label(root, bg="white", borderwidth=0)
    flash_label.place(x=0, y=0, relwidth=1, relheight=1)

    def fade_flash(alpha=1.0):
        if alpha > 0:
            gray_val = int(255 * alpha)
            color = f'#{gray_val:02x}{gray_val:02x}{gray_val:02x}'
            flash_label.config(bg=color)
            root.after(30, lambda: fade_flash(alpha - 0.25))
        else:
            flash_label.place_forget()
            flash_label.destroy()
            show_capture_preview()

    root.after(10, lambda: fade_flash(1.0))


def show_capture_preview():
    """Mostra anteprima foto scattata con istruzioni e pulsanti Annulla / Legolizza."""
    global mode
    mode = MODE_CAPTURE_PREVIEW
    frame_resized = cv2.resize(captured_frame, (PREVIEW_LIVE_W, PREVIEW_LIVE_H), interpolation=cv2.INTER_LINEAR)
    img = Image.fromarray(cv2.cvtColor(frame_resized, cv2.COLOR_BGR2RGB))
    imgtk = ImageTk.PhotoImage(img)
    preview_label.imgtk = imgtk
    preview_label.config(image=imgtk)

    status_label.config(text=STATUS_CAPTURE_PREVIEW, fg="#FFFFFF", bg="#2C3E50")
    btn_shoot.grid_remove()
    btn_cancel.grid(row=0, column=0, padx=10)
    btn_legolize.grid(row=0, column=1, padx=10)
    btn_shoot.config(state="normal")
    root.after(100, lambda: btn_legolize.focus_set())


def back_to_live_preview():
    """Torna alla schermata principale (live webcam) dall'anteprima cattura."""
    global mode
    mode = MODE_PREVIEW
    btn_cancel.grid_remove()
    btn_legolize.grid_remove()
    btn_shoot.grid(row=0, column=0, padx=10)
    status_label.config(text=STATUS_READY, fg="#FFFFFF", bg="#2C3E50")
    root.after(100, lambda: btn_shoot.focus_set())


def process_captured_photo():
    global current_photo_path, captured_frame
    frame = captured_frame
    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    # Ridimensiona a 1024x683 (3:2)
    frame_resized = cv2.resize(frame, (PHOTO_W, PHOTO_H), interpolation=cv2.INTER_LINEAR)
    input_path = os.path.join(INPUT_DIR, f"{ts}.jpg")
    cv2.imwrite(input_path, frame_resized)

    # Estensione output in base al formato Replicate
    ext = "webp" if REPLICATE_OUTPUT_FORMAT.lower() == "webp" else "jpg"
    output_path = os.path.join(OUTPUT_DIR, f"{ts}.{ext}")

    show_loading_overlay()
    root.update()

    def on_success(url):
        global current_photo_path
        try:
            download_image(url, output_path)
            img = Image.open(output_path).convert("RGB")
            img = apply_event_text_to_image(img)
            if output_path.lower().endswith(".webp"):
                img.save(output_path, format="WEBP", quality=90)
            else:
                img.save(output_path, format="JPEG", quality=95)
            current_photo_path = output_path
            # Scala di 1 le chiamate Replicate rimanenti e aggiorna label
            remaining = _decrement_replicate_remaining()
            if root.winfo_exists():
                root.after(0, lambda r=remaining: _replicate_remaining_label.config(
                    text="Legolize: %d foto rimanenti" % r
                ))
                root.after(0, lambda: _show_result(img, output_path))
        except Exception as e:
            if root.winfo_exists():
                root.after(0, lambda: _show_replicate_error(str(e)))

    def on_error(err_msg):
        if root.winfo_exists():
            root.after(0, lambda: _show_replicate_error(err_msg))

    def _show_result(image_pil, path):
        global current_photo_path
        current_photo_path = path
        show_result(image_pil)

    def _show_replicate_error(err_msg):
        hide_loading_overlay()
        status_label.config(text=STATUS_LEGO_ERROR.format(error=err_msg), fg="#FFFFFF", bg="#E74C3C")
        btn_shoot.config(state="normal")

    call_replicate_legolize(input_path, on_success, on_error)


def show_result(image_pil):
    global mode
    hide_loading_overlay()
    mode = MODE_RESULT
    imgtk = ImageTk.PhotoImage(image_pil.resize((PREVIEW_RESULT_W, PREVIEW_RESULT_H)))
    preview_label.imgtk = imgtk
    preview_label.config(image=imgtk)

    status_frame.pack_forget()
    preview_frame.pack_forget()
    btn_frame.pack_forget()
    entry_frame.pack_forget()
    btn_send.pack_forget()
    if hasattr(root, 'privacy_label') and root.privacy_label and root.privacy_label.winfo_exists():
        root.privacy_label.pack_forget()
    if hasattr(root, 'btn_action_frame') and root.btn_action_frame and root.btn_action_frame.winfo_exists():
        root.btn_action_frame.pack_forget()

    preview_frame.pack(expand=False, pady=PADDING_FRAME, padx=PADDING_FRAME)
    status_frame.pack(pady=(PADDING_FRAME, 0))
    status_label.config(text=STATUS_ENTER_EMAIL, fg="#FFFFFF", bg="#2C3E50")
    entry_frame.pack(pady=(5, 0))
    entry_email.pack(padx=40, pady=PADDING_FRAME, ipady=8)
    privacy_label = tk.Label(root, text=EMAIL_PRIVACY_NOTICE, font=("Arial", 14, "italic"),
                             fg=UI_ACCENT_COLOR, bg=UI_BG_COLOR)
    privacy_label.pack(pady=(0, PADDING_FRAME))
    root.privacy_label = privacy_label

    btn_action_frame = tk.Frame(root, bg=UI_BG_COLOR)
    btn_action_frame.pack(pady=PADDING_FRAME)
    root.btn_action_frame = btn_action_frame

    btn_send_result = tk.Button(btn_action_frame, text=BTN_SEND_TEXT,
                                font=("Arial", FONT_SIZE_BUTTON_MAIN, "bold"),
                                bg="#2196F3", fg="white", width=20, height=1, pady=5,
                                command=send_email, relief=tk.FLAT, borderwidth=0, cursor="hand2", takefocus=True)
    btn_send_result.grid(row=0, column=0, padx=5)
    btn_cancel_result = tk.Button(btn_action_frame, text=BTN_CANCEL_TEXT,
                                   font=("Arial", FONT_SIZE_BUTTON_MAIN, "bold"),
                                   bg="#757575", fg="white", width=20, height=1, pady=5,
                                   command=reset_kiosk, relief=tk.FLAT, borderwidth=0, cursor="hand2", takefocus=True)
    btn_cancel_result.grid(row=0, column=1, padx=5)
    root.btn_send_result = btn_send_result
    root.btn_cancel_result = btn_cancel_result

    def email_tab_handler(event):
        btn_send_result.focus_set()
        return "break"
    entry_email.bind('<Tab>', email_tab_handler)

    # Non nascondere entry_frame: contiene il campo email da compilare
    btn_frame.pack_forget()
    btn_cancel.grid_forget()
    btn_send.pack_forget()

    entry_email.delete(0, tk.END)
    entry_email.config(fg="#999999", highlightbackground="#E0E0E0", highlightcolor="#4CAF50", highlightthickness=3)
    entry_email.insert(0, EMAIL_PLACEHOLDER)
    root.after(100, lambda: entry_email.focus_set())
    start_inactivity_timer()
    btn_shoot.config(state="normal")


def show_engagement_screen():
    """Schermata finale come da fotomachine: successo, QR, social, scatta un'altra foto."""
    global mode
    mode = "ENGAGEMENT"
    preview_frame.pack_forget()
    entry_frame.pack_forget()
    entry_email.pack_forget()
    btn_send.pack_forget()
    btn_cancel.grid_forget()
    status_frame.pack_forget()
    btn_frame.pack_forget()
    if hasattr(root, 'privacy_label') and root.privacy_label and root.privacy_label.winfo_exists():
        root.privacy_label.pack_forget()
    if hasattr(root, 'btn_action_frame') and root.btn_action_frame and root.btn_action_frame.winfo_exists():
        root.btn_action_frame.pack_forget()

    engagement_frame = tk.Frame(root, bg=UI_BG_COLOR)
    engagement_frame.pack(expand=True, fill=tk.BOTH)

    title_frame = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    title_frame.pack(pady=30)
    emoji_success = tk.Label(title_frame, text="\u2705", font=(EMOJI_FONT, FONT_SIZE_ENGAGEMENT_TITLE),
                            fg="#FFFFFF", bg=UI_BG_COLOR)
    emoji_success.pack(side=tk.LEFT, padx=(0, 10))
    success_title = tk.Label(title_frame, text="Email inviata con successo!",
                             font=("Arial", FONT_SIZE_ENGAGEMENT_TITLE, "bold"), fg="#FFFFFF", bg=UI_BG_COLOR)
    success_title.pack(side=tk.LEFT)

    if os.path.exists(QR_CODE_PATH):
        try:
            qr_img = Image.open(QR_CODE_PATH)
            qr_img = qr_img.resize((QR_CODE_SIZE, QR_CODE_SIZE), Image.LANCZOS)
            qr_tk = ImageTk.PhotoImage(qr_img)
            qr_label = tk.Label(engagement_frame, image=qr_tk, bg=UI_BG_COLOR, borderwidth=6, relief=tk.SOLID, bd=6)
            qr_label.image = qr_tk
            qr_label.pack(pady=20)
        except Exception:
            pass

    invite_frame = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    invite_frame.pack(pady=20)
    emoji_phone = tk.Label(invite_frame, text="\U0001f4f1", font=(EMOJI_FONT, FONT_SIZE_ENGAGEMENT_TEXT),
                          fg=UI_TEXT_COLOR, bg=UI_BG_COLOR)  # mobile
    emoji_phone.pack(side=tk.LEFT, padx=(0, 10))
    invite_text = tk.Label(invite_frame, text="Scannerizza il QR code\no seguici sui social!",
                           font=("Arial", FONT_SIZE_ENGAGEMENT_TEXT, "bold"), fg=UI_TEXT_COLOR, bg=UI_BG_COLOR,
                           justify=tk.LEFT)
    invite_text.pack(side=tk.LEFT)

    social_frame_links = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    social_frame_links.pack(pady=15)
    emoji_fb = tk.Label(social_frame_links, text="\U0001f44d", font=(EMOJI_FONT, FONT_SIZE_ENGAGEMENT_SOCIAL),
                       fg=UI_ACCENT_COLOR, bg=UI_BG_COLOR)
    emoji_fb.pack(side=tk.LEFT, padx=(0, 5))
    fb_text = tk.Label(social_frame_links, text="Facebook: M9Lab",
                      font=("Arial", FONT_SIZE_ENGAGEMENT_SOCIAL), fg=UI_ACCENT_COLOR, bg=UI_BG_COLOR)
    fb_text.pack(side=tk.LEFT, padx=(0, 20))
    separator = tk.Label(social_frame_links, text="|", font=("Arial", FONT_SIZE_ENGAGEMENT_SOCIAL),
                        fg=UI_ACCENT_COLOR, bg=UI_BG_COLOR)
    separator.pack(side=tk.LEFT, padx=10)
    emoji_ig = tk.Label(social_frame_links, text="\U0001f4f8", font=(EMOJI_FONT, FONT_SIZE_ENGAGEMENT_SOCIAL),
                       fg=UI_ACCENT_COLOR, bg=UI_BG_COLOR)
    emoji_ig.pack(side=tk.LEFT, padx=(0, 5))
    ig_text = tk.Label(social_frame_links, text="Instagram: @mezzaninelab",
                      font=("Arial", FONT_SIZE_ENGAGEMENT_SOCIAL), fg=UI_ACCENT_COLOR, bg=UI_BG_COLOR)
    ig_text.pack(side=tk.LEFT)

    thanks_frame = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    thanks_frame.pack(pady=20)
    thanks_text = tk.Label(thanks_frame, text="Grazie per aver visitato il nostro stand!",
                          font=("Arial", FONT_SIZE_ENGAGEMENT_THANKS), fg=UI_TEXT_COLOR, bg=UI_BG_COLOR)
    thanks_text.pack(side=tk.LEFT, padx=(0, 10))
    emoji_rocket = tk.Label(thanks_frame, text="\U0001f680", font=(EMOJI_FONT, FONT_SIZE_ENGAGEMENT_THANKS),
                           fg=UI_TEXT_COLOR, bg=UI_BG_COLOR)
    emoji_rocket.pack(side=tk.LEFT)

    btn_frame_another = tk.Frame(engagement_frame, bg=UI_BG_COLOR)
    btn_frame_another.pack(pady=30)
    btn_another = tk.Button(btn_frame_another, text="SCATTA UN'ALTRA FOTO",
                            font=("Arial", FONT_SIZE_ENGAGEMENT_SOCIAL, "bold"),
                            bg="#4CAF50", fg="white", width=28, height=2, command=reset_kiosk,
                            relief=tk.FLAT, borderwidth=0, cursor="hand2", compound=tk.LEFT)
    try:
        btn_another.config(text="\U0001f4f8  SCATTA UN'ALTRA FOTO")
    except Exception:
        pass
    btn_another.pack()
    btn_another.focus_set()

    instruction_text = tk.Label(engagement_frame, text="Premi SPAZIO o INVIO per continuare",
                               font=("Arial", 16), fg="#FFFFFF", bg=UI_BG_COLOR)
    instruction_text.pack(pady=10)

    root.engagement_frame = engagement_frame
    root.engagement_timer = root.after(30000, reset_kiosk)


def reset_kiosk():
    global mode, inactivity_timer
    if inactivity_timer:
        root.after_cancel(inactivity_timer)
        inactivity_timer = None
    if hasattr(root, 'engagement_timer') and root.engagement_timer:
        root.after_cancel(root.engagement_timer)
        root.engagement_timer = None
    mode = MODE_PREVIEW
    if hasattr(root, 'engagement_frame'):
        root.engagement_frame.pack_forget()
    entry_frame.pack_forget()
    entry_email.pack_forget()
    btn_send.pack_forget()
    btn_cancel.grid_forget()
    if hasattr(root, 'privacy_label') and root.privacy_label and root.privacy_label.winfo_exists():
        root.privacy_label.pack_forget()
    if hasattr(root, 'btn_action_frame') and root.btn_action_frame and root.btn_action_frame.winfo_exists():
        root.btn_action_frame.pack_forget()

    def destroy_dynamic():
        if hasattr(root, 'btn_action_frame') and root.btn_action_frame:
            try:
                root.btn_action_frame.destroy()
            except Exception:
                pass
            root.btn_action_frame = None
        if hasattr(root, 'privacy_label') and root.privacy_label:
            try:
                root.privacy_label.destroy()
            except Exception:
                pass
            root.privacy_label = None
    root.after(10, destroy_dynamic)

    preview_frame.pack(expand=True, pady=PADDING_BUTTON, padx=PADDING_BUTTON)
    status_frame.pack(pady=PADDING_FRAME)
    btn_frame.pack(pady=PADDING_BUTTON)
    btn_cancel.grid_remove()
    btn_legolize.grid_remove()
    btn_shoot.grid(row=0, column=0)
    status_label.config(text=STATUS_READY, fg="#FFFFFF", bg="#2C3E50")
    btn_shoot.config(state="normal")
    root.after(100, lambda: btn_shoot.focus_set())


def start_inactivity_timer():
    global inactivity_timer
    if inactivity_timer:
        root.after_cancel(inactivity_timer)
    inactivity_timer = root.after(30000, auto_reset_on_timeout)


def auto_reset_on_timeout():
    global inactivity_timer
    inactivity_timer = None
    if mode == MODE_RESULT:
        status_label.config(text="Timeout - Ritorno alla schermata iniziale...", fg="#FFFFFF", bg="#FF9800")
        root.after(1000, reset_kiosk)


def reset_inactivity_timer(event=None):
    if mode == MODE_RESULT:
        start_inactivity_timer()


def validate_email(email):
    email_pattern = r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
    if not re.match(email_pattern, email):
        return False
    if '..' in email or email.startswith('.') or email.startswith('-'):
        return False
    if '@.' in email or '.@' in email:
        return False
    return True


def log_email_send(esito, email_address, photo_filename):
    log_file = "email_log.txt"
    try:
        contatore = 1
        if os.path.exists(log_file):
            with open(log_file, "r", encoding="utf-8") as f:
                contatore = sum(1 for _ in f) + 1
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        with open(log_file, "a", encoding="utf-8") as f:
            f.write(f"{contatore} | {timestamp} | {esito} | {photo_filename} | {email_address}\n")
        print(f"📝 Log #{contatore}: {esito} - {photo_filename} - {email_address}")
    except Exception as e:
        print(f"⚠️ Errore scrittura log: {e}")


def send_email():
    email_to = entry_email.get().strip()
    if not email_to or email_to == EMAIL_PLACEHOLDER:
        status_label.config(text=STATUS_EMAIL_REQUIRED, fg="#FFFFFF", bg="#E74C3C")
        return
    if not validate_email(email_to):
        status_label.config(text=STATUS_EMAIL_INVALID, fg="#FFFFFF", bg="#E74C3C")
        return

    entry_frame.pack_forget()
    entry_email.pack_forget()
    btn_send.pack_forget()
    btn_cancel.grid_forget()
    if hasattr(root, 'privacy_label') and root.privacy_label and root.privacy_label.winfo_exists():
        root.privacy_label.pack_forget()
    if hasattr(root, 'btn_action_frame') and root.btn_action_frame and root.btn_action_frame.winfo_exists():
        root.btn_action_frame.pack_forget()
    status_label.config(text="Invio in corso...", fg="#FFFFFF", bg="#3498DB")
    root.update()

    msg = EmailMessage()
    msg["Subject"] = EMAIL_SUBJECT
    msg["From"] = SMTP_USER
    msg["To"] = email_to
    msg.set_content("Testo alternativo per client mail che non leggono HTML")
    msg.add_alternative(EMAIL_HTML_BODY, subtype="html")

    photo_filename = os.path.basename(current_photo_path)
    subtype = "webp" if photo_filename.lower().endswith(".webp") else "jpeg"
    with open(current_photo_path, "rb") as f:
        msg.add_attachment(f.read(), maintype="image", subtype=subtype, filename=photo_filename)

    try:
        with smtplib.SMTP(SMTP_SERVER, SMTP_PORT) as server:
            server.starttls()
            server.login(SMTP_USER, SMTP_PASSWORD)
            server.send_message(msg)
        log_email_send("OK", email_to, photo_filename)
        show_engagement_screen()
    except Exception as e:
        log_email_send("KO", email_to, photo_filename)
        status_label.config(text=STATUS_EMAIL_ERROR.format(error=str(e)), fg="#FFFFFF", bg="#E74C3C")


# ================= GUI =================
root = tk.Tk()
root.configure(bg=UI_BG_COLOR)

if system == "Windows":
    try:
        root.attributes("-fullscreen", True)
    except Exception:
        w, h = root.winfo_screenwidth(), root.winfo_screenheight()
        root.geometry(f"{w}x{h}+0+0")
        root.state('zoomed')
elif system == "Linux":
    root.overrideredirect(True)
    try:
        w, h = root.winfo_screenwidth(), root.winfo_screenheight()
        root.geometry(f"{w}x{h}+0+0")
        root.lift()
        root.attributes('-topmost', True)
        root.after_idle(root.attributes, '-topmost', False)
        root.config(cursor="none")
    except Exception as e:
        print(f"⚠️ Errore fullscreen Linux: {e}")
else:
    try:
        root.attributes("-fullscreen", True)
    except Exception:
        pass

root.protocol("WM_DELETE_WINDOW", lambda: None)

canvas_bg = tk.Canvas(root, bg=UI_BG_COLOR, highlightthickness=0)
canvas_bg.place(x=0, y=0, relwidth=1, relheight=1)

# Chiamate Replicate rimanenti in alto a sinistra (piccolo, bianco)
_replicate_remaining_label = tk.Label(
    root,
    text="Legolize: %d foto rimanenti" % _get_replicate_remaining(),
    fg="#FFFFFF",
    bg=UI_BG_COLOR,
    font=("Arial", 10),
    anchor="w",
)
_replicate_remaining_label.place(x=12, y=10, anchor="nw")


def create_logo_pattern():
    try:
        if os.path.exists(UI_LOGO_PATH):
            logo_pil = Image.open(UI_LOGO_PATH).convert("RGBA")
            logo_pil = logo_pil.resize((180, 180), Image.LANCZOS)
            logo_pil = logo_pil.rotate(45, expand=True, resample=Image.BICUBIC)
            alpha = logo_pil.split()[3]
            alpha = alpha.point(lambda p: int(p * UI_LOGO_OPACITY))
            logo_pil.putalpha(alpha)
            if not hasattr(canvas_bg, 'logo_images'):
                canvas_bg.logo_images = []
            rw, rh = logo_pil.size
            sw, sh = root.winfo_screenwidth(), root.winfo_screenheight()
            for y in range(-rh, sh + rh, 220):
                for x in range(-rw, sw + rw, 220):
                    logo_tk = ImageTk.PhotoImage(logo_pil)
                    canvas_bg.create_image(x, y, image=logo_tk, anchor="nw")
                    canvas_bg.logo_images.append(logo_tk)
        else:
            print("Logo non trovato (UI_LOGO_PATH):", UI_LOGO_PATH)
    except Exception as e:
        print("Logo pattern:", e)


root.after(100, create_logo_pattern)
# Tieni il canvas dietro a tutti i widget pack/place
# Canvas.lower() è per gli item del canvas; per mandare la finestra dietro usare il comando Tk
root.after(300, lambda: root.tk.call('lower', canvas_bg._w))

preview_frame = tk.Frame(root, bg="#FFFFFF", bd=0, relief=tk.FLAT)
preview_frame.pack(expand=True, pady=PADDING_BUTTON, padx=PADDING_BUTTON)
preview_inner_frame = tk.Frame(preview_frame, bg="#E0E0E0", bd=8, relief=tk.FLAT)
preview_inner_frame.pack(padx=4, pady=4)
preview_label = tk.Label(preview_inner_frame, bg="#000000", borderwidth=0)
preview_label.pack()
root.countdown_active = False
root.countdown_number = 0

status_frame = tk.Frame(root, bg="#2C3E50", relief=tk.FLAT, borderwidth=0)
status_frame.pack(pady=PADDING_FRAME)
status_label = tk.Label(status_frame, text=STATUS_READY, fg="#FFFFFF", bg="#2C3E50",
                        font=("Arial", FONT_SIZE_STATUS, "bold"), borderwidth=0, padx=20, pady=8)
status_label.pack()

btn_frame = tk.Frame(root, bg=UI_BG_COLOR)
btn_frame.pack(pady=PADDING_BUTTON)
btn_shoot = tk.Button(btn_frame, text=BTN_SHOOT_TEXT, font=("Arial", FONT_SIZE_BUTTON_MAIN, "bold"),
                      bg="#4CAF50", fg="white", activebackground="#45a049", activeforeground="white",
                      width=20, height=1, pady=5, relief=tk.FLAT, borderwidth=0, cursor="hand2")
btn_shoot.grid(row=0, column=0, padx=10)
btn_cancel = tk.Button(btn_frame, text=BTN_CANCEL_TEXT, font=("Arial", FONT_SIZE_BUTTON_SECONDARY),
                       bg="#757575", fg="white", activebackground="#616161", activeforeground="white",
                       width=15, height=1, pady=5,
                       command=lambda: back_to_live_preview() if mode == MODE_CAPTURE_PREVIEW else reset_kiosk(),
                       relief=tk.FLAT, borderwidth=0, cursor="hand2")
btn_legolize = tk.Button(btn_frame, text=BTN_LEGOLIZE_TEXT, font=("Arial", FONT_SIZE_BUTTON_MAIN, "bold"),
                         bg="#4CAF50", fg="white", activebackground="#45a049", activeforeground="white",
                         width=20, height=1, pady=5, command=process_captured_photo,
                         relief=tk.FLAT, borderwidth=0, cursor="hand2")


def on_key_press(event):
    if event.keysym == 'Escape':
        if mode == MODE_RESULT or mode == "ENGAGEMENT":
            reset_kiosk()
        elif mode == MODE_CAPTURE_PREVIEW:
            back_to_live_preview()
        else:
            cap.release()
            root.destroy()
        return
    if mode == MODE_PREVIEW:
        if not (btn_shoot.winfo_ismapped() and btn_shoot['state'] == 'normal'):
            return
        if event.keysym in ['space', 'Space', 'Return']:
            start_countdown()
            return "break"
    elif mode == MODE_CAPTURE_PREVIEW:
        if event.keysym in ['Return', 'space', 'Space']:
            process_captured_photo()
            return "break"
    elif mode == MODE_RESULT:
        reset_inactivity_timer()
        if event.keysym == 'Return':
            focused = root.focus_get()
            btn_send_res = getattr(root, 'btn_send_result', None)
            btn_cancel_res = getattr(root, 'btn_cancel_result', None)
            if focused == entry_email or focused == btn_send_res:
                send_email()
            elif focused == btn_cancel_res:
                reset_kiosk()
    elif mode == "ENGAGEMENT":
        if event.keysym in ['space', 'Space', 'Return']:
            reset_kiosk()


root.bind('<KeyPress>', on_key_press)

entry_frame = tk.Frame(root, bg=UI_BG_COLOR)
entry_email = tk.Entry(entry_frame, font=("Arial", FONT_SIZE_EMAIL_ENTRY), width=28,
                      relief=tk.FLAT, borderwidth=0, bg="white", fg="#999999",
                      insertbackground="#333333", highlightthickness=3,
                      highlightbackground="#E0E0E0", highlightcolor="#4CAF50")


def add_placeholder(event=None):
    if entry_email.get() == "":
        entry_email.config(fg="#999999")
        entry_email.insert(0, EMAIL_PLACEHOLDER)


def remove_placeholder(event=None):
    if entry_email.get() == EMAIL_PLACEHOLDER:
        entry_email.delete(0, tk.END)
        entry_email.config(fg="#333333")


entry_email.insert(0, EMAIL_PLACEHOLDER)
entry_email.bind('<FocusIn>', remove_placeholder)
entry_email.bind('<FocusOut>', add_placeholder)
entry_email.bind('<KeyPress>', lambda e: reset_inactivity_timer())
entry_email.bind('<KeyRelease>', lambda e: reset_inactivity_timer())
entry_email.bind('<Button-1>', lambda e: reset_inactivity_timer())

btn_send = tk.Button(root, text=BTN_SEND_TEXT, font=("Arial", FONT_SIZE_BUTTON_MAIN, "bold"),
                     bg="#2196F3", fg="white", activebackground="#1976D2", activeforeground="white",
                     width=25, height=1, pady=5, command=send_email,
                     relief=tk.FLAT, borderwidth=0, cursor="hand2")
btn_send.bind('<Button-1>', lambda e: reset_inactivity_timer())
btn_cancel.bind('<Button-1>', lambda e: reset_inactivity_timer())

btn_exit = tk.Button(root, text=BTN_EXIT_TEXT, font=("Arial", 14, "bold"),
                     bg="#E53935", fg="white", activebackground="#C62828", activeforeground="white",
                     command=lambda: (cap.release(), root.destroy()),
                     relief=tk.FLAT, borderwidth=0, padx=20, pady=10, cursor="hand2")
btn_exit.place(relx=0.98, rely=0.02, anchor="ne")

btn_shoot.config(command=start_countdown)
root.after(200, lambda: btn_shoot.focus_set())

update_preview()
root.mainloop()
