/*
 * WGS84Point.h
 *
 *  Created on: Oct 15, 2014
 *      Author: ivp
 */
#pragma once
#include <tmx/messages/message.hpp>

namespace tmx::messages{

	/// WGS84Point encapsulates coordinates on the earths surface.
	struct WGS84Position
	{
		WGS84Position() : Latitude(0), Longitude(0), Elevation(0) {}

		WGS84Position(double latitude, double longitude, double elevation = 0.0):
			Latitude(latitude), Longitude(longitude), Elevation(elevation) { }

		static message_tree_type to_tree(const WGS84Position& pos){
            message_tree_type tree;
            tree.put("latitude", pos.Latitude);
            tree.put("longitude", pos.Longitude);
            tree.put("elevation", pos.Elevation);
            return tree;
        }
        static WGS84Position from_tree(const message_tree_type& tree){
            WGS84Position pos;
            pos.Latitude = tree.get<double>("latitude");
            pos.Longitude = tree.get<double>("longitude");
            pos.Elevation = tree.get<double>("elevation");
            return pos;
        }
        /// Latitude in degrees
		double Latitude;
        /// Longitude in degrees
		double Longitude;
        /// Elevation in meters
		double Elevation;
	};

}

