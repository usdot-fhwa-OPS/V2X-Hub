#pragma once

namespace tmx::utils {


    /// Three dimensional Vector
    typedef struct Vector3d
    {
        Vector3d() : X(0), Y(0), Z(0) {}

        Vector3d(double x, double y, double z = 0.0):
            X(x), Y(y), Z(z) { }

        double X;
        double Y;
        double Z;
    } Vector3d;

} // namespace tmx::utils
