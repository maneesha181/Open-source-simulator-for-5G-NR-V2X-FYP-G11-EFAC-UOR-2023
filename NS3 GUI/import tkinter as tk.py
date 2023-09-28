import tkinter as tk
import subprocess
from datetime import datetime

class OsmWebWizardApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Open Source Simulator UI for 5G NR V2X Mode 2 - FYP 2023")

        self.create_ui()

    def create_ui(self):
        self.label = tk.Label(self.root, text="Welcome To Open Source Simulator UI for 5G NR V2X Mode 2 - FYP 2023")
        self.label.pack(pady=10, padx=10, anchor="w")

        self.output_text = tk.Text(self.root, height=10, width=60)
        self.output_text.pack(padx=10, pady=5, anchor="w")

        self.run_button = tk.Button(self.root, text="(1) Run osmWebWizard", command=self.run_command)
        self.run_button.pack(pady=10, padx=10, anchor="e")

    def run_command(self):
        try:
            process = subprocess.Popen(
                ["python3", "/home/manee/sumo/tools/osmWebWizard.py"],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )

            while process.poll() is None:
                line = process.stdout.readline()
                if line:
                    self.output_text.insert("end", line)
                    self.output_text.see("end")
                self.root.update_idletasks()

            output, errors = process.communicate()
            if output:
                if self.output_text.get("end-1c", "end") != "\n":
                    self.output_text.insert("end", "\n")
                self.output_text.insert("end", output)
            else:
                self.output_text.insert("end", "Step 01 | Success | osmWebWizard UI opened!")

            if errors:
                self.output_text.insert("end", errors)

            self.save_output_to_file(output + errors)
        except subprocess.CalledProcessError as e:
            error_message = f"Error: {e.returncode}\n{e.output.decode('utf-8')}"
            self.show_error_message(error_message)

    def save_output_to_file(self, output):
        timestamp = datetime.now().strftime("%Y%m%d%H%M%S")
        filename = f"output_{timestamp}.txt"
        with open(filename, "w") as file:
            file.write(output)
        self.output_text.insert("end", f"\nOutput saved to: {filename}")

    def show_error_message(self, message):
        error_window = tk.Toplevel(self.root)
        error_window.title("Error")
        error_label = tk.Label(error_window, text=message)
        error_label.pack(padx=10, pady=10)

def main():
    root = tk.Tk()
    app = OsmWebWizardApp(root)
    root.mainloop()

if __name__ == "__main__":
    main()
