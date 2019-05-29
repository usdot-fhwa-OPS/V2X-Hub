import tkinter as tk
from tkinter import filedialog as fd
import os, sys, statistics, gmplot, geojson, webbrowser, csv


class Browse(tk.Frame):
    """ Creates a frame that contains a button when clicked lets the user to select
    a file and put its filepath into an entry.
    """

    def __init__(self, master, initialdir='', filetypes=()):
        super().__init__(master)
        self.filepath = tk.StringVar()
        self._initaldir = initialdir
        self._filetypes = filetypes
        self._widgets()        
        
    def _widgets(self):
        self._logo_path = r"C:\Users\ghiasia\Documents\Projects\18 - 327 OpsIV V2X Hub\Logo2.png"
        self._logo_load = tk.PhotoImage(file = self._logo_path)
        self._logo = tk.Label(self, image = self._logo_load)
        
        self._entry = tk.Entry(self, textvariable=self.filepath,width=40,state='disabled')    
        self._button = tk.Button(self, text="Browse...", command=self.browse,font=("Arial Bold", 10))
        self._label_folder = tk.Label(self, text="Input Folder:",font=("Arial Bold", 10))
        self._label_empty1 = tk.Label(self, text="",font=("Arial Bold", 12))
        self._label_empty2 = tk.Label(self, text="",font=("Arial Bold", 12))
        self._label_tt = tk.Label(self, text="Travel times:",font=("Arial Bold", 12))
        self._label_delay = tk.Label(self, text="Average delay:",font=("Arial Bold", 12))
        self._label_other = tk.Label(self, text="Other measures ...",font=("Arial Bold", 12))
        self._label_bsm1 = tk.Label(self, text="Number of BSM data recoreds:",font=("Arial Bold", 10))
        
        self._logo.grid(column=0, row=0)
        self._label_empty1.grid(column=0, row=1)
        self._label_folder.grid(column=1, row=3)
        self._button.grid(column=1, row=4)
        self._entry.grid(column=3, row=4)
        self._label_empty1.grid(column=0, row=5)
        self._label_tt.grid(column=3, row=7)
        self._label_delay.grid(column=3, row=8)
        self._label_other.grid(column=3, row=9)
        self._label_bsm1.grid(column=1, row=5)

    def browse(self):
        self.filepath.set(fd.askdirectory(initialdir=self._initaldir))
        self.plot_map()
        self.read_bsm()

    def plot_map(self):
        file_address = self.filepath.get()
        for file in os.listdir(file_address): 
             if file.endswith(".geojson"):
                file = open(file_address + '/' + file)
                break
        map_data = geojson.load(file)
        num_points = len(map_data["features"])
        lat = []
        lon = []
        for i in range(num_points):
            lat.append(map_data["features"][i]["geometry"]["coordinates"][1])
            lon.append(map_data["features"][i]["geometry"]["coordinates"][0])
        
        lat_mean = statistics.mean(lat)
        lon_mean = statistics.mean(lon)
        gmap = gmplot.GoogleMapPlotter(lat_mean,lon_mean, 18) 
        gmap.apikey = 'AIzaSyAQQJkyargtjRhmz8gh15IZFuiWbCrYdT0'
        gmap.__format__ = 'SATELLITE'
        # here, we plot two sets of scatter points: 1- all points, 2- a subsample for boundary selection
        # there should be a number beside each of the subsample points so that the user can select from
        gmap.scatter( lat, lon, '#ff0000',size = 3, marker = False ) 
        gmap.marker(lon_mean, lon_mean, 'cornflowerblue')
        gmap.draw( "my_map.html" ) 
        webbrowser.open("my_map.html",new=2)
        
    def read_bsm(self):
        self.bsm = []
        address = self.filepath.get()
        for file in os.listdir(address + '\BSMs'):
            file_address = address + '/BSMs/' + file
            with open(file_address) as csv_file:
                csv_reader = csv.reader(csv_file)
                # This skips the first row of the CSV file.
                next(csv_reader)
                # using the list function is faster, but the list structure is harder to work with
                for row in csv_reader:
                    self.bsm.append(row) 
        # printing the number of bsm records  
        self._label_bsm2 = tk.Label(self, text=len(self.bsm),font=("Arial Bold", 10))
        self._label_bsm2.grid(column=1, row=6)
        
if __name__ == '__main__':
    root = tk.Tk()
    root.title("V2X Hub Performance Measures")
    # root.geometry('325x250')
    root.configure(background = "gray")
    
    file_browser = Browse(root, initialdir=r"C:\Users\ghiasia\Documents\Projects\18 - 327 OpsIV V2X Hub\Metrics\Code",
                                filetypes=(('GeoJSON','*.geojson'),("All files", "*.*")))
    file_browser.pack(fill='x', expand=True)

    root.mainloop()
    
    