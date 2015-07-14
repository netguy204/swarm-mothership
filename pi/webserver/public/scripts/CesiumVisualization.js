define("CesiumVisualization", ["Cesium/Cesium"], function (Cesium) {

	var hunterEntities = [];
	var obstructions = [];
	var addHunter = function (pid, location) {
		console.log("adding hunter", location);
		hunterEntities[pid] = viewer.entities.add({
				position : Cesium.Cartesian3.fromDegrees(location.latitude, location.longitude),
				point : {
					pixelSize : 10,
					color : Cesium.Color.BLUE
				}
			});
	}

	viewer = new Cesium.Viewer('cesiumContainer');

	var layers = viewer.imageryLayers;

	//39.167840, -76.899314 northwest
	//39.166930, -76.896176 southeast

	layers.addImageryProvider(new Cesium.SingleTileImageryProvider({
			url : '../images/testSite.png',
			rectangle : Cesium.Rectangle.fromDegrees(-76.899314,39.166930,-76.896176, 39.167840)
		}));

	//Test site -76.897721, 39.167282
	//Makerspace -76.898080, 39.165368
	viewer.camera.flyTo({
		destination : Cesium.Cartesian3.fromDegrees(-76.897721, 39.167282, 120)
	});

	return {
		resize : function () {},

		updateHunter : function (pid, location) {
			if (hunterEntities[pid] == null || hunterEntities[pid] == undefined) {
				addHunter(pid, location);
				return;
			}
			hunterEntities[pid].position = Cesium.Cartesian3.fromDegrees(location.latitude, location.longitude);
		},

		addObstruction : function (location) {}

	};
});
