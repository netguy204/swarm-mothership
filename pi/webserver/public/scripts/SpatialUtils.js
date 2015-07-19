define("SpatialUtils", ["Geodesy/latlon-vincenty"], function (LatLon) {

	//Ranges are reported in centimetres

	var getReadoutIndexHeading = function (heading, index) {
		//starts at right which is heading + 45 degrees
		return heading + (-45 + index);
	}

	return {
		rangesToLines : function (location, heading, ranges) {
			var p1 = new LatLon(location["latitude"], location["longitude"]);
			var vertices = [];
			var line = [];
			for (var i = 0; i < ranges.length; i++) {
				//break the line and start another if range exceeds ultrasonics
				if(ranges[i] > 500){
					if(line.length > 4){
						vertices.push(line);
						line = [];
					}
				}
				else{
				var p2 = p1.destinationPoint(ranges[i] / 100, getReadoutIndexHeading(heading, i));
				line.push(p2.lon);
				line.push(p2.lat);
				}
			}
			if(line.length > 4){
				vertices.push(line);
			}
			console.log("ranges to lines", vertices);
			return vertices;

		},

		getBeaconLineVertices : function (entityLocation, entityHeading, beaconReadout) {
			var p1 = new LatLon(entityLocation["latitude"], entityLocation["longitude"]);
			var vertices = [];

			for (var i = 0; i < beaconReadout.length; i++) {
				if (beaconReadout[i] === true) {
					var line = [];
					line.push(entityLocation["longitude"]);
					line.push(entityLocation["latitude"]);
					
					//Whatever the max effective range of the IR sensor is, it should be where the 0.5 is.
					var p2 = p1.destinationPoint(0.5, entityHeading + (-45 + i));
					line.push(p2.lon);
					line.push(p2.lat);
					
					vertices.push(line);
				}
			}
			return vertices;
		},

		debugRangesToLines : function () {
			var location = {
				lat : 39.1672858,
				lon : -76.8976622
			};
			var heading = 45;
			var ranges = [0,25,500,1000,5000,10000];

			var p1 = new LatLon(location.lat, location.lon);
			var destinationPoints = [];
			
			for (var range in ranges) {
				var p2 = p1.destinationPoint(ranges[range] / 100, heading); // p2.toString(): 37.6528°S, 143.9265°E
				console.log(ranges[range] + " centimetres", p2);
				destinationPoints.push(p2);
			}
			
			for(var destination in destinationPoints){
				console.log(p1.distanceTo(destinationPoints[destination]));
			}
			
			
		}
	};

});
