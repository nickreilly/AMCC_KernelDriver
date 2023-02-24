import tkinter as tk
from tkinter import ttk
import time

filename = '/home/dsp/Desktop/LatestInfo.txt'

def update_text():
    with open(filename, "r") as f:
        content = f.read()
    label.config(text=content)
    root.after(1000, update_text)

def stop():
    root.destroy()

def update_progress_label():
    return f"Current Progress: {pb['value']}%"

def progress():
    if pb['value'] < 100:
        pb['value'] += 20
        value_label['text'] = update_progress_label()
    else:
        showinfo(message='The progress completed!')

root = tk.Tk()
root.title('Last Acquisition Values')
# root.geometry()
label = tk.Label(root, text="")
label.pack()

stop_button = tk.Button(root, text="Stop", command=stop)
stop_button.pack(side="bottom")

update_text()


# progressbar- Not working yet
pb = ttk.Progressbar(
    root,
    orient='horizontal',
    mode='determinate',
    length=280
)


root.mainloop()