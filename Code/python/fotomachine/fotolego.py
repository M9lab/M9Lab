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

# ================= CARICAMENTO CONFIG DA .ENV =================
load_dotenv()

# Dimensioni
PREVIEW_W = int(os.getenv("PREVIEW_W", 640))
PREVIEW_H = int(os.getenv("PREVIEW_H", 480))
PHOTO_W = int(os.getenv("PHOTO_W", 1920))
PHOTO_H = int(os.getenv("PHOTO_H", 1080))
COUNTDOWN_SECONDS = int(os.getenv("COUNTDOWN_SECONDS", 3))
HAAR_FILE = os.getenv("HAAR_FILE", "haarcascade_frontalface_default.xml")

# Cartelle input/output
INPUT_DIR = os.getenv("INPUT_DIR", "input_photos")
OUTPUT_DIR = os.getenv("OUTPUT_DIR", "output_photos")

# Variabile per testo evento
EVENT_TEXT = os.getenv("EVENT_TEXT", "Benvenuti al LEGO Museum! #LEGOTrains")

# Finestra (poster) path
WINDOW_TEMPLATE_PATH = os.getenv("WINDOW_TEMPLATE_PATH", "window.png")
WINDOW_POS = (
    int(os.getenv("WINDOW_POS_X", 1000)),
    int(os.getenv("WINDOW_POS_Y", 50))
)
WINDOW_SIZE = (
    int(os.getenv("WINDOW_SIZE_W", 400)),
    int(os.getenv("WINDOW_SIZE_H", 300))
)

# Email config
SMTP_SERVER = os.getenv("SMTP_SERVER", "smtp.gmail.com")
SMTP_PORT = int(os.getenv("SMTP_PORT", 587))
SMTP_USER = os.getenv("SMTP_USER", "")
SMTP_PASSWORD = os.getenv("SMTP_PASSWORD", "")
EMAIL_SUBJECT = os.getenv("EMAIL_SUBJECT", "La tua foto LEGO")
EMAIL_HTML_BODY = os.getenv("EMAIL_HTML_BODY", "<h2>Grazie per aver partecipato!</h2><p>Ecco la tua foto LEGO dall'evento.</p>")

# ================= GLOBAL =================
MODE_PREVIEW = 0
MODE_RESULT = 1
mode = MODE_PREVIEW
current_photo_path = None

# ================= CREAZIONE CARTELLE =================
os.makedirs(INPUT_DIR, exist_ok=True)
os.makedirs(OUTPUT_DIR, exist_ok=True)

# ================= HAAR CASCADE =================
if not os.path.exists(HAAR_FILE):
    urllib.request.urlretrieve(
        "https://raw.githubusercontent.com/opencv/opencv/master/data/haarcascades/haarcascade_frontalface_default.xml",
        HAAR_FILE
    )
face_cascade = cv2.CascadeClassifier(HAAR_FILE)

# ================= WEBCAM =================
cap = cv2.VideoCapture(0, cv2.CAP_DSHOW)
if not cap.isOpened():
    raise RuntimeError("Errore apertura fotocamera")
else:
    print("Webcam OK")

# ================= FUNZIONI =================
def legoify(face, block=12):
    img = Image.fromarray(cv2.cvtColor(face, cv2.COLOR_BGR2RGB))
    small = img.resize((img.width // block, img.height // block), Image.NEAREST)
    return small.resize(img.size, Image.NEAREST)

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
            img = Image.fromarray(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
            imgtk = ImageTk.PhotoImage(img)
            preview_label.imgtk = imgtk
            preview_label.config(image=imgtk)
    root.after(50, update_preview)

def start_countdown():
    btn_shoot.config(state="disabled")
    overlay_label.place(relx=0.5, rely=0.5, anchor="center")
    countdown_step(COUNTDOWN_SECONDS)

def countdown_step(sec):
    if sec > 0:
        overlay_label.config(text=str(sec))
        root.after(1000, lambda: countdown_step(sec - 1))
    else:
        overlay_label.place_forget()
        capture_photo()

def capture_photo():
    global mode, current_photo_path
    ret, frame = cap.read()
    if not ret:
        status_label.config(text="Errore fotocamera")
        btn_shoot.config(state="normal")
        return

    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    input_path = os.path.join(INPUT_DIR, f"{ts}.png")
    output_path = os.path.join(OUTPUT_DIR, f"{ts}.jpg")

    # Salva foto originale
    cv2.imwrite(input_path, frame)

    # Alta qualità per merge
    frame_hd = cv2.resize(frame, (PHOTO_W, PHOTO_H), interpolation=cv2.INTER_CUBIC)
    gray = cv2.cvtColor(frame_hd, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray, 1.1, 5)
    frame_pil = Image.fromarray(cv2.cvtColor(frame_hd, cv2.COLOR_BGR2RGB))

    if len(faces) > 0:
        x, y, w, h = faces[0]
        lego_face = legoify(frame_hd[y:y+h, x:x+w])
        frame_pil.paste(lego_face, (x, y))

    # ================= Composizione finale con window.png =================
    if os.path.exists(WINDOW_TEMPLATE_PATH):
        window_bg = Image.open(WINDOW_TEMPLATE_PATH).convert("RGBA")
        guest_img = resize_to_fit(frame_pil, WINDOW_SIZE).convert("RGBA")
        composed = window_bg.copy()
        composed.paste(guest_img, WINDOW_POS, guest_img)
        final_image = composed.convert("RGB")
    else:
        final_image = frame_pil.convert("RGB")

    # Testo evento
    draw = ImageDraw.Draw(final_image)
    try:
        font = ImageFont.truetype("arial.ttf", 48)
    except:
        font = ImageFont.load_default()
    bbox = draw.textbbox((0,0), EVENT_TEXT, font=font)
    text_w = bbox[2] - bbox[0]
    text_h = bbox[3] - bbox[1]
    draw.text(((PHOTO_W - text_w)//2, PHOTO_H - 100), EVENT_TEXT, fill="yellow", font=font)

    # Salva JPEG compressa ma alta qualità
    final_image.save(output_path, format="JPEG", quality=95)
    current_photo_path = output_path
    show_result(final_image)

def show_result(image_pil):
    global mode
    mode = MODE_RESULT
    imgtk = ImageTk.PhotoImage(image_pil.resize((PREVIEW_W, PREVIEW_H)))
    preview_label.imgtk = imgtk
    preview_label.config(image=imgtk)
    status_label.config(text="Inserisci email o annulla")
    btn_shoot.grid_forget()
    btn_cancel.grid(row=0, column=0)
    entry_email.pack(pady=5)
    btn_send.pack(pady=10)

def reset_kiosk():
    global mode
    mode = MODE_PREVIEW
    entry_email.pack_forget()
    btn_send.pack_forget()
    btn_cancel.grid_forget()
    btn_shoot.grid(row=0, column=0)
    status_label.config(text="Premi SCATTA FOTO LEGO")
    btn_shoot.config(state="normal")

def send_email():
    email_to = entry_email.get()
    if not email_to:
        status_label.config(text="Inserisci un'email valida")
        return

    msg = EmailMessage()
    msg["Subject"] = EMAIL_SUBJECT
    msg["From"] = SMTP_USER
    msg["To"] = email_to
    msg.set_content("Testo alternativo per client mail che non leggono HTML")
    msg.add_alternative(EMAIL_HTML_BODY, subtype="html")

    with open(current_photo_path, "rb") as f:
        file_data = f.read()
        msg.add_attachment(file_data, maintype="image", subtype="jpeg", filename=os.path.basename(current_photo_path))

    try:
        with smtplib.SMTP(SMTP_SERVER, SMTP_PORT) as server:
            server.starttls()
            server.login(SMTP_USER, SMTP_PASSWORD)
            server.send_message(msg)
        status_label.config(text="Email inviata con successo!")
        root.after(3000, reset_kiosk)  # reset automatico
    except Exception as e:
        status_label.config(text=f"Errore invio email: {e}")

# ================= GUI =================
root = tk.Tk()
root.attributes("-fullscreen", True)
root.configure(bg="black")
root.protocol("WM_DELETE_WINDOW", lambda: None)

preview_label = tk.Label(root, bg="black")
preview_label.pack(expand=True)
overlay_label = tk.Label(root, fg="white", bg="black", font=("Arial", 120, "bold"))
status_label = tk.Label(root, text="Premi SCATTA FOTO LEGO", fg="white", bg="black", font=("Arial", 20))
status_label.pack(pady=10)

btn_frame = tk.Frame(root, bg="black")
btn_frame.pack(pady=10)

btn_shoot = tk.Button(btn_frame, text="SCATTA FOTO LEGO", font=("Arial", 26, "bold"), bg="#4CAF50", fg="white", width=20, height=2)
btn_shoot.grid(row=0, column=0, padx=10)

btn_cancel = tk.Button(btn_frame, text="ANNULLA", font=("Arial", 20), bg="#757575", fg="white", width=15, height=2, command=reset_kiosk)

entry_email = tk.Entry(root, font=("Arial", 22), width=30)
btn_send = tk.Button(root, text="INVIA FOTO", font=("Arial", 24, "bold"), bg="#2196F3", fg="white", width=25, height=2, command=send_email)

btn_exit = tk.Button(root, text="ESCI", font=("Arial", 14), bg="#b71c1c", fg="white", command=lambda: (cap.release(), root.destroy()))
btn_exit.place(relx=0.98, rely=0.02, anchor="ne")

# ================= AVVIO =================
btn_shoot.config(command=start_countdown)
update_preview()
root.mainloop()
