using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using System.Windows.Media;

namespace V2IHubSimulator
{
    public class Simulation
    {
        //key is vehicle ID
        public Dictionary<int, List<SimulationWaypoint>> WaypointLists;
        public Dictionary<int, Color> Colors;
        public Dictionary<int, int> Clones;
        public Dictionary<int, double> CloneOffsets;

        public string Name { get; set; }
        public double Duration_seconds { get;  set; }

        public int NumberOfVehicles { get; set; }

        public Simulation ()
        {
            WaypointLists = new Dictionary<int, List<SimulationWaypoint>>();
            Colors = new Dictionary<int, Color>();
            Clones = new Dictionary<int, int>();
            CloneOffsets = new Dictionary<int, double>();
            NumberOfVehicles = 0;
        } 

        public void SetWaypointSpeed(int vehicleId, int wpNum, double speed_mph)
        {
            WaypointLists[vehicleId][wpNum].Speed_mph = speed_mph;
        }

        public void SetWaypointPause(int vehicleId, int wpNum, double pause_seconds)
        {
            WaypointLists[vehicleId][wpNum].Pause_seconds = pause_seconds;
        }

        public static void SerializeToFile(Simulation scenario, string filename)
        {
            JsonSerializerSettings settings = new JsonSerializerSettings();
            settings.ReferenceLoopHandling = ReferenceLoopHandling.Ignore;
            string jsonString = JsonConvert.SerializeObject(scenario, settings);

            System.IO.File.WriteAllText(filename, jsonString);
        }

        public static Simulation SimulationFromFile(string filename)
        {
            string jsonString = System.IO.File.ReadAllText(filename);

            Simulation tempSimulation = JsonConvert.DeserializeObject<Simulation>(jsonString);

            return tempSimulation;

        }
    }
}
