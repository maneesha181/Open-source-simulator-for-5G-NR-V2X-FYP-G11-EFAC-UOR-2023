import tkinter as tk
import subprocess
from datetime import datetime

class OsmWebWizardApp:
    def __init__(self, root):
        self.root = root
        self.root.title("OSM Web Wizard")

        self.create_ui()

    def create_ui(self):
        self.label = tk.Label(self.root, text="Click the button to run 'python3 osmWebWizard.py'")
        self.label.pack(pady=10)

        self.output_text = tk.Text(self.root, height=10, width=40)
        self.output_text.pack(padx=10, pady=5)

        self.run_button = tk.Button(self.root, text="Run Command", command=self.run_command)
        self.run_button.pack()

    def run_command(self):
        try:
            result = subprocess.run(["python3", "osmWebWizard.py"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
            output = result.stdout + result.stderr
            self.output_text.insert("end", output)
            self.save_output_to_file(output)
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
