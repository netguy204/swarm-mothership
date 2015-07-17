requirejs(["MothershipControl","CesiumVisualization","FakeHunter","StatusBox"], function(MothershipControl,Visualization,FakeHunter,StatusBox) {
	
	var hunters = [];
	var obstructions = [];
	
	var updatePositions = function(updateResponse){
		var platforms = JSON.parse(updateResponse);
		for(var platform in platforms){
			Visualization.updateHunter(platforms[platform]);
		}
		StatusBox.updateStatus(platforms);
	};
	
	var statusUpdateInterval = setInterval(function(){
	
		var req = new XMLHttpRequest();
		req.onload = function(evt){
			updatePositions(req.response);
		};
		req.withCredentials = true;
		req.open("GET", "/status", true);
		req.send();
	},2000);
	
	
	window.onresize = function(){
	}
	
	//var fh = new FakeHunter();
	//fh.start();

	
	var controlInput = new MothershipControl();
	controlInput.setEntityPID(0);
	
	
});