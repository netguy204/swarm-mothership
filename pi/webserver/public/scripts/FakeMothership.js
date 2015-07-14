define("FakeMothership", ["Proj4"], function (Proj4) {
	//EPSG3559
	var lat = -76.898085;
	var lon = 39.165372;
	var currentTaskID = 0;
	var pid = 0;
	var speedMetersPerSecond = 0.3;

	// creating source and destination Proj4js objects
	// once initialized, these may be re-used as often as needed
	var source = new Proj4.Proj(‘EPSG : 4326’); //source coordinates will be in Longitude/Latitude
	var dest = new Proj4.Proj(‘EPSG : 3559’); //destination coordinates in NAD83

	var getNextTask = function () {
		var req = new XMLHttpRequest();
		req.onload = function (evt) {
			var task = JSON.parse(req.response);
			if (task.length > 0 && task[0].cid != undefined) {
				currentTaskID = task[0].cid;
				simulateTaskCompletion();
				return;
			}
			setTimeout(getNextTask, 1000);
		};
		req.withCredentials = true;
		req.open("GET", "http://localhost:8080/commands?pid=" + pid, true);
		req.send();
	};

	var postStatusUpdate = function () {
		console.log("post status update", this);

		var status = {
			"pid" : 1,
			"latitude" : lat + (Math.random() * 0.00001),
			"longitude" : lon + (Math.random() * 0.00001),
			"beacon" : [0, 0, 0, 0, 0, 0],
			"obstruction" : [0, 0, 0, 0, 0, 0]
		};
		var req = new XMLHttpRequest();
		req.withCredentials = true;
		req.open("PUT", "http://localhost:8080/status", true);
		req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
		req.send(JSON.stringify(status));
	};

	var putTaskCompleted = function () {
		var req = new XMLHttpRequest();
		req.onload = function (evt) {
			getNextTask();
		};
		var status = {
			"cid" : currentTaskID,
			"pid" : 1,
			"complete" : true
		};
		req.withCredentials = true;
		req.open("PUT", "http://localhost:8080/commands", true);
		req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
		req.send(JSON.stringify(status));
		postStatusUpdate();
	};

	var simulateTaskCompletion = function () {
		setTimeout(putTaskCompleted, 1000);
	};

	return function () {
		return {
			start : function () {
				postStatusUpdate();
			}
		};
	};
});
