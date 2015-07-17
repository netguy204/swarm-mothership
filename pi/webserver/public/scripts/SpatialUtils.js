define("SpatialUtils", ["Geodesy/latlon-vincenty"], function (LatLon) {

	//Ranges are reported in centimetres
	
	var getReadoutIndexHeading = function(heading,index){
		//starts at right which is heading + 45 degrees
		return heading + (-45 + index);
	}

		return {
			rangesToLines : function (location, heading, ranges) {
				var p1 = new LatLon(location["latitude"],location["longitude"]);
				var vertices = [];
				
				for(var i=0;i<ranges.length;i++){
						var p2 = p1.destinationPoint(ranges[i]/100,getReadoutIndexHeading(heading,i));
						vertices.push(p2.lat);
						vertices.push(p2.lon);
					}
				return vertices;
				
			},
			
			debugRangesToLines : function () {
				var location = {
					lat : "0",
					lon : "0"
				};
				var heading = 0;
				var ranges = [0, 5, 10, 1000];

				var p1 = new LatLon(location.lat, location.lon);
			
				for (var range in ranges) {
					var p2 = p1.destinationPoint(ranges[range] / 100, heading); // p2.toString(): 37.6528°S, 143.9265°E
					console.log(ranges[range] + " centimetres", p2);
				}
			}
		};

});
