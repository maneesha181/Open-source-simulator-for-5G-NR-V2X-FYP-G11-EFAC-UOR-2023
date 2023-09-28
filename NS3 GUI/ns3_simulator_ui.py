import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
import subprocess
from datetime import datetime
import os
import signal

class OsmWebWizardApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Open Source Simulator for 5G NR V2X Mode 2 Communication - FYP 2023")
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

        self.create_ui()

    def create_ui(self):
        self.heading_label = tk.Label(self.root, text="Open Source Simulator for 5G NR V2X Mode 2 Communication - FYP 2023", font=("Georgia", 22, "bold"))
        self.heading_label.grid(row=0, column=0, columnspan=4, pady=20, padx=20, sticky="nsew")  # Updated sticky parameter

        

        self.group_label = tk.Label(self.root, text="Supervised by: Dr. Geeth Priyankara \n\nGroup Members: Dissanayaka D.M.S.D., Nethmini K.G.M., Dharmarathne R.G.C.D.S.\n\nFaculty of Engineering, University of Ruhuna", font=("Georgia", 11))
        self.group_label.grid(row=1, column=0, columnspan=4, pady=20, padx=20, sticky="nsew")

        self.start_button = tk.Button(self.root, text="Step 02: Start Generate Trace file", command=self.start_process_1, font=("Georgia", 10, "bold"))
        self.start_button.grid(row=2, column=1, pady=20, padx=20, sticky="w")

        self.start_button = tk.Button(self.root, text="Step 03: Mobility file Process", command=self.start_process_2, font=("Georgia", 10, "bold"))
        self.start_button.grid(row=2, column=2, pady=20, padx=40, sticky="w")

        self.start_button = tk.Button(self.root, text="Step 03: --shift", command=self.start_process_3, font=("Georgia", 10, "bold"))
        self.start_button.grid(row=2, column=3, pady=20, padx=20, sticky="w")

        #self.start_button = tk.Button(self.root, text="Step 04: ns3 result", command=self.start_process_4)
        #self.start_button.grid(row=2, column=3, pady=20, padx=20, sticky="e")

        self.num_veh_label = tk.Label(self.root, text="Number of Vehicles:", font=("Georgia", 10))
        self.num_veh_label.grid(row=3, column=0, pady=10, padx=30, sticky="nsew")
        self.num_veh_entry = tk.Entry(self.root)
        self.num_veh_entry.grid(row=3, column=1, pady=10, padx=10, sticky="w")

        self.tracefile_var = tk.IntVar()
        self.tracefile_checkbox = tk.Checkbutton(self.root, text="Enable Trace File", variable=self.tracefile_var)
        self.tracefile_checkbox.grid(row=3, column=2, pady=10, padx=10, sticky="w")

     
        self.sim_label = tk.Label(self.root, text="Run Simulation:", font=("Georgia", 10))
        self.sim_label.grid(row=4, column=0, pady=10, padx=30, sticky="nsew")
        
        self.start_button = tk.Button(self.root, text="Broadcast excute", command=self.start_process_4, font=("Georgia", 10))
        
        self.start_button.grid(row=4, column=1, pady=20, padx=20, sticky="e")
        self.start_button = tk.Button(self.root, text="Unicast excute", command=self.start_process_5, font=("Georgia", 10))
        
        self.start_button.grid(row=4, column=2, pady=20, padx=180, sticky="e")
        self.start_button = tk.Button(self.root, text="Groupcast excute", command=self.start_process_6,font=("Georgia", 10))
        
        self.start_button.grid(row=4, column=3, pady=20, padx=20, sticky="w")
        
        
        self.exit_button = tk.Button(self.root, text="Exit", command=self.root.quit, font=("Georgia", 10, "bold"))
        self.exit_button.grid(row=7, column=3, pady=20, padx=20, sticky="e")
        
         # Create a frame to contain the Text widget and scrollbar
        text_frame = tk.Frame(self.root)
        text_frame.grid(row=6, column=0, columnspan=4, padx=20, pady=10, sticky="nsew")

        # Create a Text widget and scrollbar
        self.output_text = tk.Text(text_frame, height=14, width=120)
        self.output_text.grid(row=0, column=0, sticky="nsew")

        text_scrollbar = tk.Scrollbar(text_frame, command=self.output_text.yview)
        text_scrollbar.grid(row=0, column=1, sticky="ns")
        
        self.output_text.config(yscrollcommand=text_scrollbar.set)

        text_frame.grid_rowconfigure(0, weight=1)
        text_frame.grid_columnconfigure(0, weight=1)

       

    def start_process_1(self):
        try:
            result = subprocess.run(["sumo", "-c","/home/manee/ns3-ui-new/sumoOutput/osm.sumocfg","--fcd-output","sumoOutput/trace.xml"],
             stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
            output = result.stdout + result.stderr
            self.output_text.insert("end", output)
            self.save_output_to_file(output)
        except subprocess.CalledProcessError as e:
            error_message = f"Error: {e.returncode}\n{e.output.decode('utf-8')}"
            self.show_error_message(error_message)

    def start_process_2(self):
        try:
            #python traceExporter.py -i 2023-08-13-11-15-06/trace.xml --ns2mobility-output=2023-08-13-11-15-06/mobility.tcl
            result = subprocess.run(["python3", "/home/manee/sumo/tools/traceExporter.py","-i","sumoOutput/trace.xml","--ns2mobility-output=/home/manee/ns3-ui-new/sumoOutput/mobility.tcl"],
             stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
            output = result.stdout + result.stderr
            self.output_text.insert("end", output)
            self.output_text.insert("end", "Step 03 | Success | mobility process finished!\n")
            self.save_output_to_file(output)
        except subprocess.CalledProcessError as e:
            error_message = f"Error: {e.returncode}\n{e.output.decode('utf-8')}"
            self.show_error_message(error_message)

    def start_process_3(self):
        try:
            #python traceExporter.py -i 2023-08-13-11-15-06/trace.xml --ns2mobility-output=2023-08-13-11-15-06/mobility.tcl
            result = subprocess.run(["python3", "/home/manee/sumo/tools/traceExporter.py","-i","sumoOutput/trace.xml","--ns2mobility-output=/home/manee/ns3-ui-new/sumoOutput/mobility.tcl","--shift=1"],
             stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
            output = result.stdout + result.stderr
            self.output_text.insert("end", output)
            self.output_text.insert("end", "Step 03 | Success | mobility process finished!\n")
            self.save_output_to_file(output)
        except subprocess.CalledProcessError as e:
            error_message = f"Error: {e.returncode}\n{e.output.decode('utf-8')}"
            self.show_error_message(error_message)

    def start_process_4(self):
        try:
            num_veh = self.num_veh_entry.get()
            tracefile_enabled = self.tracefile_var.get()

            

            if tracefile_enabled:
                command = [
                "/home/manee/Downloads/nrRepo/ns-3-dev/./ns3",
                "run",
                f"scratch/nr-v2x-examples-broadcast/cttc-nr-v2x-demo-simple --numVeh={num_veh} --tracefile=/home/manee/ns3-ui-new/sumoOutput/mobility.tcl"
            ]
            else:
                command = [
                "/home/manee/Downloads/nrRepo/ns-3-dev/scratch/nr-v2x-examples-broadcast/./ns3",
                "run",
                f"scratch/nr-v2x-examples-broadcast/cttc-nr-v2x-demo-simple --numVeh={num_veh}"
            ]
            
            
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
            output = result.stdout + result.stderr
            self.output_text.insert("end", output)
            self.output_text.insert("end", "\nStep 04 | Success | Broadcast ns3 Run Finished!\n")
            self.save_output_to_file(output)
        except subprocess.CalledProcessError as e:
            error_message = f"Error: {e.returncode}\n{e.output.decode('utf-8')}"
            self.show_error_message(error_message)
            
            
    def start_process_5(self):
        try:
            num_veh = self.num_veh_entry.get()
            tracefile_enabled = self.tracefile_var.get()

            

            if tracefile_enabled:
                command = [
                "/home/manee/Downloads/nrRepo/ns-3-dev/./ns3",
                "run",
                f"scratch/nr-v2x-examples-unicast/cttc-nr-v2x-demo-simple --numVeh={num_veh} --tracefile=/home/manee/ns3-ui-new/sumoOutput/mobility.tcl"
            ]
            else:
                command = [
                "/home/manee/Downloads/nrRepo/ns-3-dev/scratch/nr-v2x-examples-unicast/./ns3",
                "run",
                f"scratch/nr-v2x-examples-unicast/cttc-nr-v2x-demo-simple --numVeh={num_veh}"
            ]
            
            
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
            output = result.stdout + result.stderr
            self.output_text.insert("end", output)
            self.output_text.insert("end", "\nStep 04 | Success | Unicast ns3 Run Finished!\n")
            self.save_output_to_file(output)
        except subprocess.CalledProcessError as e:
            error_message = f"Error: {e.returncode}\n{e.output.decode('utf-8')}"
            self.show_error_message(error_message)
            
            
            
    def start_process_6(self):
        try:
            num_veh = self.num_veh_entry.get()
            tracefile_enabled = self.tracefile_var.get()

            

            if tracefile_enabled:
                command = [
                "/home/manee/Downloads/nrRepo/ns-3-dev/./ns3",
                "run",
                f"scratch/nr-v2x-examples-groupcast/cttc-nr-v2x-demo-simple --numVeh={num_veh} --tracefile=/home/manee/ns3-ui-new/sumoOutput/mobility.tcl"
            ]
            else:
                command = [
                "/home/manee/Downloads/nrRepo/ns-3-dev/scratch/nr-v2x-examples-groupcast/./ns3",
                "run",
                f"scratch/nr-v2x-examples-groupcast/cttc-nr-v2x-demo-simple --numVeh={num_veh}"
            ]
            
            
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
            output = result.stdout + result.stderr
            self.output_text.insert("end", output)
            self.output_text.insert("end", "\nStep 04 | Success | Groupcast ns3 Run Finished!\n")
            self.save_output_to_file(output)
        except subprocess.CalledProcessError as e:
            error_message = f"Error: {e.returncode}\n{e.output.decode('utf-8')}"
            self.show_error_message(error_message)
            
            
            
            
            
            

    def save_output_to_file(self, output):
        timestamp = datetime.now().strftime("%Y%m%d%H%M%S")
        filename = f"output_{timestamp}.txt"
        with open(filename, "w") as file:
            file.write(output)
        self.output_text.insert("end", f"\nOutput saved to: {filename}\n\n")

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
