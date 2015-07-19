requirejs(["MothershipControl","CesiumVisualization","FakeHunter","StatusBox"], function(MothershipControl,Visualization,FakeHunter,StatusBox) {
	
	var hunters = {};
	var obstructions = [];
	
	var updatePositions = function(updateResponse){
		var platforms = JSON.parse(updateResponse);
		for(var platform in platforms){
			var p = platforms[platform];
			if(hunters[p.pid] !== undefined){
				if(hunters[p.pid].mtime !== p.mtime){
					Visualization.updateHunter(p);
				}
			}
			else{
				hunters[p.pid] = p;
				Visualization.updateHunter(p);
			}
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
	
	var simulatedHunters = [];
	for(var i=0;i<4;i++){
	simulatedHunters.push(new FakeHunter(i + 100,{latitude:39.1672858 + (Math.random() * 0.00004) ,longitude: -76.8976622 + (Math.random() * 0.000004)}));
	simulatedHunters[i].start();
	simulatedHunters[i].getProperties();
	}

	
	var controlInput = new MothershipControl();
	controlInput.setEntityPID(0);
	
	
});