define("CesiumVisualization",["Cesium/Cesium"],function(Cesium){

var hunterEntities = [];

setTimeout(function(){ console.log(Cesium)},5000);

viewer = new Cesium.Viewer('cesiumContainer');


viewer.camera.flyTo({
	destination: Cesium.Cartesian3.fromDegrees(-76.896736,39.170863,300)
});

return{
	resize:function(){
	
	},

	addHunter: function(pid,location){
		hunterEntities[pid] = viewer.entities.add({
			position: Cesium.Cartesian3.fromDegrees(location.longitude,location.latitude),
			point: {
			pixelSize: 10,
			color: Cesium.Color.BLUE
			}
		});
	},
	
	updateHunter: function(pid,location){
		console.log(hunterEntities[pid]);
	}
	
};
});