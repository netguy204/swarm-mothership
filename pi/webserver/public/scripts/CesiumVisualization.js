define("CesiumVisualization", ["Cesium/Cesium", "SpatialUtils"], function (Cesium, SpatialUtils) {
	var viewer,
	layers;

	viewer = new Cesium.Viewer('cesiumContainer', {
			imageryProvider : new Cesium.SingleTileImageryProvider({
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
	var irPositives = [];

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

	var addObstructionLine = function (entityLocation, entityHeading, rangeReadout) {
	if(entityLocation === undefined || entityHeading === undefined || rangeReadout === undefined){
		return;
	}
	try{
	var vertices = SpatialUtils.rangesToLines(entityLocation, entityHeading, rangeReadout);
	var positions = Cesium.Cartesian3.fromDegreesArray(vertices);
		obstructions.push(
		viewer.entities.add({
			name : 'Obstruction',
			polyline : {
				positions : positions,
				width : 5,
				material : Cesium.Color.BLACK
			}
		}));
		}
		catch(err){console.error(err);}
	};
	
	var addIrPositives = function(entityLocation,entityHeading,beaconReadout){
		
	};

	var addHunter = function (pid, location) {
		console.log("adding hunter", location);
		if(pid === 0){
			hunterEntities[pid] = viewer.entities.add({
				position : Cesium.Cartesian3.fromDegrees(location.latitude, location.longitude),
				point : {
					pixelSize : 14,
					color : Cesium.Color.GREEN,
				}
			});
		}
		hunterEntities[pid] = viewer.entities.add({
				position : Cesium.Cartesian3.fromDegrees(location.latitude, location.longitude),
				point : {
					pixelSize : 10,
					color : Cesium.Color.BLUE,
				}
			});
			
	};

	//39.167840, -76.899314 northwest
	//39.166930, -76.896176 southeast


	//Test site -76.897721, 39.167282
	//Makerspace -76.898080, 39.165368
	viewer.camera.flyTo({
		destination : Cesium.Cartesian3.fromDegrees(-76.897721, 39.167282, 120)
	});

	return {
		resize : function () {},

		updateHunter : function (entity) {
			var location = {
				"latitude" : entity.lat,
				"longitude" : entity["long"]
			}

			if (hunterEntities[entity.pid] == null || hunterEntities[entity.pid] == undefined) {
				addHunter(entity.pid, location);
				//addObstructionLine(location, entity.heading, entity.obstruction);
				return;
			}
			hunterEntities[entity.pid].position = Cesium.Cartesian3.fromDegrees(location.latitude, location.longitude);
			hunterEntities[entity.pid].name = entity.pid + " - Battery: " + entity.Vbattery;
			addObstructionLine(location, entity.heading, entity.obstruction);
		}

	};
});
