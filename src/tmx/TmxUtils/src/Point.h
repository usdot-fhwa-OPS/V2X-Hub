#pragma once

namespace tmx::utils {


    /// Cartesian Coordinates on a .
    typedef struct Point
    {
        Point() : X(0), Y(0), Z(0) {}

        Point(double x, double y, double z = 0.0):
            X(x), Y(y), Z(z) { }

        double X;
        double Y;
        double Z;
    } Point;

} // namespace tmx::utils
