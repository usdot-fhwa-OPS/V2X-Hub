using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Maps.MapControl.WPF;

namespace V2IHubSimulator
{
    public class SimulationWaypointSegment : MapPolyline
    {
        public SimulationWaypoint ParentWaypoint { get; private set; }

        public SimulationWaypointSegment(SimulationWaypoint wp)
        {
            ParentWaypoint = wp;
        }
    }
}
