define("CesiumVisualization", ["Cesium/Cesium"], function (Cesium) {

	var hunterEntities = [];
	var obstructions = [];
	var addHunter = function(pid,location){
	console.log("adding hunter", location);
	hunterEntities[pid] = viewer.entities.add({
					position : Cesium.Cartesian3.fromDegrees(location.latitude,location.longitude),
					point : {
						pixelSize : 10,
						color : Cesium.Color.BLUE
					}
				});
	}
	
	viewer = new Cesium.Viewer('cesiumContainer');

	viewer.camera.flyTo({
		destination : Cesium.Cartesian3.fromDegrees(-76.898080, 39.165368, 300)
	});

	return {
		resize : function () {},
		
		updateHunter : function (pid, location) {
			if (hunterEntities[pid] == null || hunterEntities[pid] == undefined) {
				addHunter(pid,location);
				return;
			}
			hunterEntities[pid].position = Cesium.Cartesian3.fromDegrees(location.latitude,location.longitude);
		},
		
		addObstruction: function(location){
		
		}

	};
});
