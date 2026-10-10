import tkinter as tk
from tkinter import ttk, messagebox
import subprocess
import re

ENGINE_EXECUTABLE = "./classroom_engine.exe"

class ClassroomAllocatorUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Smart Classroom Allocation & Reallocation System")
        self.root.geometry("800x650")
        self.root.configure(padx=20, pady=20)

        style = ttk.Style()
        style.theme_use('clam')
        style.configure("TFrame", background="#f4f4f4")
        style.configure("TLabel", background="#f4f4f4", font=("Segoe UI", 10))
        style.configure("Header.TLabel", font=("Segoe UI", 14, "bold"), padding=(0, 10))

        self.create_widgets()

    def create_widgets(self):
        control_frame = ttk.Frame(self.root)
        control_frame.pack(fill=tk.X, side=tk.TOP, pady=(0, 20))

        # --- Initial Allocation ---
        alloc_frame = ttk.LabelFrame(control_frame, text=" Initial Class Booking ")
        alloc_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 10))

        ttk.Label(alloc_frame, text="Class ID:").grid(row=0, column=0, padx=5, pady=5, sticky=tk.W)
        self.alloc_class_id = ttk.Entry(alloc_frame, width=10)
        self.alloc_class_id.grid(row=0, column=1, padx=5, pady=5)

        ttk.Label(alloc_frame, text="Room ID:").grid(row=1, column=0, padx=5, pady=5, sticky=tk.W)
        self.alloc_room_id = ttk.Entry(alloc_frame, width=10)
        self.alloc_room_id.grid(row=1, column=1, padx=5, pady=5)

        ttk.Label(alloc_frame, text="Time [Start, End):").grid(row=2, column=0, padx=5, pady=5, sticky=tk.W)
        time_frame_1 = ttk.Frame(alloc_frame)
        time_frame_1.grid(row=2, column=1, sticky=tk.W)
        self.alloc_start = ttk.Entry(time_frame_1, width=4)
        self.alloc_start.pack(side=tk.LEFT, padx=(5,2))
        self.alloc_end = ttk.Entry(time_frame_1, width=4)
        self.alloc_end.pack(side=tk.LEFT, padx=(2,5))

        ttk.Button(alloc_frame, text="Book Class", command=self.book_class).grid(row=3, column=0, columnspan=2, pady=10)

        # --- Reallocation Engine ---
        realloc_frame = ttk.LabelFrame(control_frame, text=" Dynamic Reallocation Engine ")
        realloc_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(10, 0))

        ttk.Label(realloc_frame, text="Class ID:").grid(row=0, column=0, padx=5, pady=5, sticky=tk.W)
        self.realloc_class_id = ttk.Entry(realloc_frame, width=10)
        self.realloc_class_id.grid(row=0, column=1, padx=5, pady=5)

        ttk.Label(realloc_frame, text="Req. Capacity:").grid(row=1, column=0, padx=5, pady=5, sticky=tk.W)
        self.realloc_cap = ttk.Entry(realloc_frame, width=10)
        self.realloc_cap.grid(row=1, column=1, padx=5, pady=5)

        ttk.Label(realloc_frame, text="Time [Start, End):").grid(row=2, column=0, padx=5, pady=5, sticky=tk.W)
        time_frame_2 = ttk.Frame(realloc_frame)
        time_frame_2.grid(row=2, column=1, sticky=tk.W)
        self.realloc_start = ttk.Entry(time_frame_2, width=4)
        self.realloc_start.pack(side=tk.LEFT, padx=(5,2))
        self.realloc_end = ttk.Entry(time_frame_2, width=4)
        self.realloc_end.pack(side=tk.LEFT, padx=(2,5))

        # Projector Checkbox
        self.realloc_proj_var = tk.BooleanVar()
        ttk.Checkbutton(realloc_frame, text="Requires Projector", variable=self.realloc_proj_var).grid(row=3, column=0, columnspan=2, pady=5, sticky=tk.W, padx=5)

        ttk.Button(realloc_frame, text="Trigger AI Reallocation", command=self.reallocate_class).grid(row=4, column=0, columnspan=2, pady=10)

        # --- Console Output ---
        console_frame = ttk.Frame(self.root)
        console_frame.pack(fill=tk.BOTH, expand=True)
        
        ttk.Label(console_frame, text="System Log & AI Output", style="Header.TLabel").pack(anchor=tk.W)
        
        self.console = tk.Text(console_frame, bg="#1e1e1e", fg="#00ff00", font=("Consolas", 10), state=tk.DISABLED)
        self.console.pack(fill=tk.BOTH, expand=True)

    def log_message(self, message):
        self.console.config(state=tk.NORMAL)
        self.console.insert(tk.END, message + "\n")
        self.console.see(tk.END)
        self.console.config(state=tk.DISABLED)

    def process_result(self, stdout_text):
        match = re.search(r"RESULT_ROOM:(.+)", stdout_text)
        if match:
            room = match.group(1).strip()
            if room == "FAILED":
                messagebox.showerror("Allocation Failed", "Could not allocate class.\nNo suitable room found or schedule conflict.")
            else:
                messagebox.showinfo("Success!", f"Class successfully assigned to Room: {room}")

    def run_backend_command(self, args):
        command = [ENGINE_EXECUTABLE] + args
        try:
            result = subprocess.run(command, capture_output=True, text=True, check=True)
            self.log_message(f"> {' '.join(command)}")
            
            # Hide the RESULT_ROOM tag from the UI console log, but print everything else
            clean_output = re.sub(r"RESULT_ROOM:.*\n?", "", result.stdout)
            if clean_output.strip():
                self.log_message(clean_output.strip())
                
            self.process_result(result.stdout)
        except FileNotFoundError:
            self.log_message(f"ERROR: Backend '{ENGINE_EXECUTABLE}' not found. Did you compile it?")
        except subprocess.CalledProcessError as e:
            self.log_message(f"> {' '.join(command)}")
            self.log_message(f"ERROR executing command:\n{e.stderr}")

    def book_class(self):
        c_id = self.alloc_class_id.get()
        r_id = self.alloc_room_id.get()
        start = self.alloc_start.get()
        end = self.alloc_end.get()
        
        if not all([c_id, r_id, start, end]):
            messagebox.showwarning("Input Error", "Please fill all booking fields.")
            return

        self.log_message(f"\n--- Attempting to Book Class {c_id} ---")
        self.run_backend_command(["--book", str(c_id), str(r_id), str(start), str(end)])

    def reallocate_class(self):
        c_id = self.realloc_class_id.get()
        cap = self.realloc_cap.get()
        start = self.realloc_start.get()
        end = self.realloc_end.get()
        proj = "1" if self.realloc_proj_var.get() else "0"

        if not all([c_id, cap, start, end]):
            messagebox.showwarning("Input Error", "Please fill all reallocation fields.")
            return

        self.log_message(f"\n--- Triggering Reallocation for Class {c_id} ---")
        self.run_backend_command(["--realloc", str(c_id), str(cap), proj, str(start), str(end)])

if __name__ == "__main__":
    root = tk.Tk()
    app = ClassroomAllocatorUI(root)
    root.mainloop()