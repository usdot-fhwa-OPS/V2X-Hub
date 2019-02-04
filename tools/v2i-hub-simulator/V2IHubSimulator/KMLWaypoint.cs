using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace V2IHubSimulator
{
    class KMLWaypoint
    {
        public int VehicleID { get; set; }
        public double Speed_mph { get; set; }
        public double Latitude { get; set; }
        public double Longitude { get; set; }
        public double Heading { get; set; }
        public long Tick { get; set; }
    }
}
