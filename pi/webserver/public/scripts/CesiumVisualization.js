define("CesiumVisualization", ["Cesium/Cesium", "SpatialUtils"], function (Cesium, SpatialUtils) {
	var viewer,layers;
	
	viewer = new Cesium.Viewer('cesiumContainer',{
	imageryProvider: new Cesium.SingleTileImageryProvider({
			url : '../images/testSite.png',
			rectangle : Cesium.Rectangle.fromDegrees(-76.899314, 39.166930, -76.896176, 39.167840)
		}),
		baseLayerPicker : false
	});

	//layers = viewer.imageryLayers;
	/*layers.addImageryProvider(new Cesium.SingleTileImageryProvider({
			url : '../images/testSite.png',
			rectangle : Cesium.Rectangle.fromDegrees(-76.899314, 39.166930, -76.896176, 39.167840)
		}));*/

	var hunterEntities = [];
	var obstructions = [];

	var getObstacleHeight = function () {
		var heights = [];
		for (var i = 0; i < 90; i++) {
			heights.push(5);
		}
		return heights;
	};

	var standardHeight = getObstacleHeight();
	var minHeight = standardHeight.map(function (g) {
			return g * 0
		});

	var addObstruction = function (entityLocation, entityHeading, rangeReadout) {
		var vertices = SpatialUtils.rangesToLines(entityLocation, entityHeading, rangeReadout);

		obstructions.add(viewer.entities.add({
				wall : {
					positions : Cesium.Cartesian3.fromDegreesArray(vertices),
					maximumHeights : standardHeight,
					minimumHeights : minHeight,
					outline : true,
					outlineColor : Cesium.Color.LIGHTGRAY,
					outlineWidth : 4,
					material : Cesium.Color.fromRandom({
						alpha : 0.7
					})
				}
			}));
	};

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

	//39.167840, -76.899314 northwest
	//39.166930, -76.896176 southeast


	//Test site -76.897721, 39.167282
	//Makerspace -76.898080, 39.165368
	viewer.camera.flyTo({
		destination : Cesium.Cartesian3.fromDegrees(-76.897721, 39.167282, 120)
	});

	return {
		resize : function () {},

		updateHunter : function (pid, location, heading, obstructions) {
			if (hunterEntities[pid] == null || hunterEntities[pid] == undefined) {
				addHunter(pid, location);
				addObstruction(location, heading, obstructions);
				return;
			}
			hunterEntities[pid].position = Cesium.Cartesian3.fromDegrees(location.latitude, location.longitude);
			addObstruction(location, heading, obstructions);
		},

		addObstruction : function (location) {}

	};
});
