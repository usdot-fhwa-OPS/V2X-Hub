using Microsoft.Maps.MapControl.WPF;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace V2IHubSimulator
{
    public class SimulationWaypoint
    {
        public int VehicleID { get; set; }
        public int WaypointNumber { get; set; }
        public double Speed_mph { get; set; }
        public double Pause_seconds { get; set; }

        public double Latitude { get; set; }

        public double Longitude { get; set; }

        public Brush Background { get; set; }

        public SimulationWaypointPushPin GetPushPin()
        {
            SimulationWaypointPushPin pp = new SimulationWaypointPushPin(this);
            pp.Location = new Location(Latitude, Longitude);
            pp.Content = WaypointNumber + 1;
            pp.Background = Background;
            pp.Foreground = new SolidColorBrush(Colors.Black);
            return pp;
        }

        public SimulationWaypointSegment GetPolyline(double startLatitude, double startLongitude)
        {
            SimulationWaypointSegment pl = new SimulationWaypointSegment(this);
            LocationCollection locs = new LocationCollection();
            locs.Add(new Location(startLatitude, startLongitude));
            locs.Add(new Location(Latitude, Longitude));
            pl.Locations = locs;
            pl.Stroke = Background;
            pl.StrokeThickness = 2;
            return pl;
        }
    }
}
