import tkinter as tk
import subprocess
from datetime import datetime
import os
import shutil
import signal


class OsmWebWizardApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Sumo Generate GUI - FYP 2023")
  # self.root.geometry("1024x680")
        self.root.grid_rowconfigure(0, weight=1)
        self.root.grid_rowconfigure(1, weight=1)
        self.root.grid_rowconfigure(2, weight=1)
        self.root.grid_rowconfigure(3, weight=1)
        self.root.grid_rowconfigure(6, weight=1)
        self.root.grid_columnconfigure(0, weight=1)
        self.root.grid_columnconfigure(1, weight=1)
        self.root.grid_columnconfigure(2, weight=1)
        self.root.grid_columnconfigure(3, weight=1)
        self.process = None
        #self.output_dir = "/home/manee/ns3-ui-new/sumoOutput"
        self.create_ui()
        self.remove_folder()

    def create_ui(self):
        self.heading_label = tk.Label(self.root, text="Sumo Mobility Generation", font=("Georgia", 24, "bold"))
        self.heading_label.grid(row=0, column=0, columnspan=4, pady=20, padx=20, sticky="nsew")

        
        self.group_label = tk.Label(self.root, text="Supervised by: Dr. Geeth Priyankara \n\nGroup Members: Dissanayaka D.M.S.D., Nethmini K.G.M., Dharmarathne R.G.C.D.S.\n\nDepartment of Electrical and Information Engineering, \nFaculty of Engineering, University of Ruhuna", font=("Georgia", 12))
        self.group_label.grid(row=1, column=2, pady=20, padx=20, sticky="nsew")

        #self.output_text = tk.Text(self.root, height=15, width=120)
        #self.output_text.grid(row=2,column=0, columnspan=1, padx=20, pady=10, sticky="nsew")

        self.start_button = tk.Button(self.root, text="Step 01: Start OSMWebWizard", command=self.start_process, font=("Georgia", 10, "bold"))
        self.start_button.grid(row=2, column=3, pady=20, padx=20, sticky="e")

        self.kill_button = tk.Button(self.root, text="Kill Process", command=self.kill_process, font=("Georgia", 10, "bold"))
        self.kill_button.grid(row=7, column=0, pady=20, padx=20, sticky="w")

        self.exit_button = tk.Button(self.root, text="Exit", command=self.root.quit, font=("Georgia", 10, "bold"))
        self.exit_button.grid(row=7, column=3, pady=20, padx=20, sticky="e")
        
        text_frame = tk.Frame(self.root)
        text_frame.grid(row=6, column=0, columnspan=4, padx=20, pady=10, sticky="nsew")

        # Create a Text widget and scrollbar
        self.output_text = tk.Text(text_frame, height=12, width=120)
        self.output_text.grid(row=0, column=0, sticky="nsew")

        text_scrollbar = tk.Scrollbar(text_frame, command=self.output_text.yview)
        text_scrollbar.grid(row=0, column=1, sticky="ns")
        
        self.output_text.config(yscrollcommand=text_scrollbar.set)

        text_frame.grid_rowconfigure(0, weight=1)
        text_frame.grid_columnconfigure(0, weight=1)
        
    def remove_folder(self):
        folder_path = "/home/manee/ns3-ui-new/sumoOutput"
        try:
        	shutil.rmtree(folder_path)
        	#print(f"Folder at deleted successfully.")
        except OSError as e:
        	print(f"Error deleting folder: {e}")


    def start_process(self):
     
    	if self.process is None:

            	try:
                	self.process = subprocess.Popen(
                    #["python3", "/home/manee/sumo/tools/osmWebWizard.py","-o", self.output_dir],
                   ["python3", "/home/manee/sumo/tools/osmWebWizard.py","-o", "sumoOutput"],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True,
                    preexec_fn=os.setsid  # Create a new process group, so we can kill the entire group later
                )
                	self.output_text.insert("end", "Step 01 | Success | OSMWebWizard process started!\n")
                	self.start_button.config(state="disabled")
            	except subprocess.CalledProcessError as e:
                	error_message = f"Error: {e.returncode}\n{e.output.decode('utf-8')}"
      				#self.show_error_message(error_message)


    def kill_process(self):
        if self.process is not None:
            try:
                os.killpg(os.getpgid(self.process.pid), signal.SIGTERM)
                self.output_text.insert("end", "Step | Success | OSMWebWizard process killed!\n")
                self.start_button.config(state="normal")
            except Exception as e:
                self.show_error_message(str(e))
            finally:
                self.process = None

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
