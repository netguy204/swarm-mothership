define("SpatialUtils", ["Geodesy/latlon-vincenty"], function (LatLon) {

	//Ranges are reported in centimetres in a

		return {
			rangesToLines : function (location, heading, ranges) {
				var p1 = new LatLon(location["latitude"],location["longitude"]);
				var vertices = [];
				for(var range in ranges){
					var p2 = p1.destinationPoint(ranges[range]/100,heading);
					vertices.push(p2.lon);
					vertices.push(p2.lat);
					//vertices.push(100);//arbitrary height meters for heights
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
