define("FakeMothership", function () {
	
	var lon = -76.8978759;
	var lat = 39.1672858;
	var currentTaskID = 0;
	var pid = 0;
	var speedMetersPerSecond = 0.3;

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
		req.open("GET", "/commands?pid=" + pid, true);
		req.send();
	};

	var postStatusUpdate = function () {
		var status = {
			"pid" : pid,
			"lat" : lat + (Math.random() * 0.00001),
			"long" : lon + (Math.random() * 0.00001),
		};
		var req = new XMLHttpRequest();
		req.withCredentials = true;
		req.open("PUT", "/status", true);
		req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
		req.send(JSON.stringify(status));
	};

	var putTaskCompleted = function () {
		var req = new XMLHttpRequest();
		req.onload = function (evt) {
			//getNextTask();
		};
		var status = {
			"cid" : currentTaskID,
			"pid" : pid,
			"complete" : true
		};
		req.withCredentials = true;
		req.open("PUT", "/commands", true);
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
