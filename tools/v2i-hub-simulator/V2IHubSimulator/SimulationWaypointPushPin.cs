using Microsoft.Maps.MapControl.WPF;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace V2IHubSimulator
{
    public class SimulationWaypointPushPin :Pushpin
    {
        public SimulationWaypoint ParentWaypoint { get; private set; }

        public SimulationWaypointPushPin(SimulationWaypoint wp)
        {
            ParentWaypoint = wp;
        }
    }
}
