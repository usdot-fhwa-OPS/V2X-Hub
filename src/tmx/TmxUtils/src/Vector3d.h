#pragma once

namespace tmx::utils {


    /// Three dimensional Vector
    using Vector3d = struct Vector3d
    {
        Vector3d() : X(0), Y(0), Z(0) {}

        Vector3d(double x, double y, double z = 0.0):
            X(x), Y(y), Z(z) { }

        double X;
        double Y;
        double Z;
    };

} // namespace tmx::utils
