/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2017, Torc Robotics, LLC
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Torc Robotics, LLC nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include "wgs84_utils.h"

#include <cmath>
/**
 * @brief gets the meters per radian of latitude at a global coordinate
 * @param tie_point
 * @return
 */
double wgs84_utils::calcMetersPerRadLat(const wgs84_utils::wgs84_coordinate &tie_point)
{
    double s_sq = pow(sin(tie_point.lat * DEG2RAD), 2);

    double R_m = Rea * (1.0 - e_sqr) / pow(sqrt(1.0 - e_sqr * s_sq), 3);

    double meters_per_rad_lat = R_m + tie_point.elevation;

    return meters_per_rad_lat;
}

/**
 * @brief gets the meters per radian of longitude at a global coordinate
 * @param tie_point
 * @return
 */
double wgs84_utils::calcMetersPerRadLon(const wgs84_utils::wgs84_coordinate &tie_point)
{
    double s_sq = pow(sin(tie_point.lat * DEG2RAD), 2);

    double R_n = Rea / (sqrt(1.0 - e_sqr * s_sq));

    double meters_per_rad_lon = (R_n + tie_point.elevation) * cos(tie_point.lat * DEG2RAD);

    return meters_per_rad_lon;
}


