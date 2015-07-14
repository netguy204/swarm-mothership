requirejs(["MothershipControl","CesiumVisualization","FakeHunter"], function(MothershipControl,Visualization,FakeHunter) {
	
	var hunters = [];
	var obstructions = [];
	
	var updatePositions = function(updateResponse){
		var platforms = JSON.parse(updateResponse);
		for(var platform in platforms){
			Visualization.updateHunter(platforms[platform].pid,{latitude:platforms[platform].latitude,longitude:platforms[platform].longitude});
		}
	};
	
	var statusUpdateInterval = setInterval(function(){
	
		var req = new XMLHttpRequest();
		req.onload = function(evt){
			updatePositions(req.response);
		};
		req.withCredentials = true;
		req.open("GET", "http://localhost:8080/status", true);
		req.send();
	},2000);
	
	
	window.onresize = function(){
		Visualization.resize();
	}
	
	var fh = new FakeHunter();
	fh.start();
	//Visualization.addHunter(1,{longitude:-76.896736,latitude:39.170863});
	
});