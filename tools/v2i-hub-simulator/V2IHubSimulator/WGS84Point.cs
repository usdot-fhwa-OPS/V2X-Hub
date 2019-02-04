using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace V2IHubSimulator
{
    class WGS84Point
    {
        public double Latitude;
        public double Longitude;
        public double Elevation;
        public WGS84Point()
        {
            Latitude = 0.0;
            Longitude = 0.0;
            Elevation = 0.0;
        }
        public WGS84Point(double latitude, double longitude, double elevation = 0.0)
        {
            Latitude = latitude;
            Longitude = longitude;
            Elevation = elevation;
        }
    }

}
