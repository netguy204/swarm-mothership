define("FakeHunter",["proj4"],function(Proj4){

var lat = -76.896736;
var lon = 39.170863;
var currentTaskPID = 0;

var postFakeCommandFromMothership = function(){
	var left = Math.ceil(Math.random * 3);
	var right = Math.ceil(Math.random * 3);

	var req = new XMLHttpRequest();
	req.onload = function(response){
		console.log("sentTask to queue",response)
	};
		req.withCredentials = true;
		req.open("POST", "http://localhost:8080/commands", true);
	    command = {
			left: left,
			right:right,
			duration: 0.25
		};
		req.send(command);
}

var getNextTask = function(){
	var req = new XMLHttpRequest();
	req.onload = function(response){
		console.log("gotTask",response);
	};
		req.withCredentials = true;
		req.open("GET", "http://localhost:8080/commands?pid=" + currentTaskPID, true);
		req.send();
}

var putTaskCompleted = function(){
	currentTaskPID++;
	var req = new XMLHttpRequest();
	req.onload = function(response){
		console.log("send task completion",response);
		getNextTask();
		};
		var status = {
		cid: currentTaskPID,
		complete: true,
		latitude: lat += (Math.random * 0.001),
		longitude: lon += (Math.random * 0.001)
		};
		req.withCredentials = true;
		req.open("PUT", "http://localhost:8080/commands", true);
		req.send();
}

var simulateTaskCompletion = function(){
	setTimeout(putTaskCompleted,1000);
}



return{
	start: function(){
		setInterval(postFakeCommandFromMothership,2000);
		setInterval(getNextTask,1000);
	},
	getPosition: function(){
	return {latitude:lat,longitude:lon};
	}
};
});