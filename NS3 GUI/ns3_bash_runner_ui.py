import tkinter as tk
from tkinter import messagebox
import subprocess

class BashScriptRunnerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Bash Script Runner")

        self.scripts = [
            ("Script 1", "script1.sh"),
            ("Script 2", "script2.sh"),
            ("Script 3", "script3.sh"),
            # Add more script buttons and paths as needed
        ]

        self.label = tk.Label(root, text="Click the buttons to run each script:")
        self.label.pack(pady=10)

        self.script_buttons = []
        for label, script_path in self.scripts:
            button = tk.Button(root, text=label, command=lambda path=script_path: self.run_script(path))
            button.pack()
            self.script_buttons.append(button)

        self.quit_button = tk.Button(root, text="Quit", command=root.quit)
        self.quit_button.pack(pady=10)

    def run_script(self, script_path):
        try:
            subprocess.run(["bash", script_path], check=True)
            messagebox.showinfo("Script Completed", f"Script {script_path} completed successfully.")
        except subprocess.CalledProcessError:
            messagebox.showerror("Script Error", f"Script {script_path} encountered an error.")

def main():
    root = tk.Tk()
    app = BashScriptRunnerApp(root)
    root.mainloop()

if __name__ == "__main__":
    main()
