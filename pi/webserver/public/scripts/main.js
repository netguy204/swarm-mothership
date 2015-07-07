requirejs(["MothershipControl","CesiumVisualization","FakeHunter"], function(MothershipControl,Visualization,FakeHunter) {
	
	var hunters = [];
	var obstructions = [];
	
	var updatePositions = function(updateResponse){
		console.log(updateResponse);
	};
	
	var statusUpdateInterval = setInterval(function(){
	
		var req = new XMLHttpRequest();
		req.onload = function(response){
			updatePositions(response);
		};
		req.withCredentials = true;
		req.open("GET", "http://localhost:8080/status", true);
		req.send();
	},2000);
	
	
	window.onresize = function(){
		Visualization.resize();
	}
	
	FakeHunter.start();
	//Visualization.addHunter(1,{longitude:-76.896736,latitude:39.170863});
	
});