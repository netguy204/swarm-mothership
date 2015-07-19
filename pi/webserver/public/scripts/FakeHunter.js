define("FakeHunter", [], function () {
	var lon = -76.8976622;
	var lat = 39.1672858;
	var currentTaskID = 0;
	var vBattery = 3.4;

	var pid = 1;
	
	var genarateFakeObstructionData = function(){
		var readings = [];
		for(var i=0;i<90;i++){
			readings.push(Math.random() * 3000);
		}
		return readings;
	}
	
	var generateFakeBeaconData = function(){
		var readings = [];
		for(var i = 0;i<90;i++){
			if(Math.round(Math.random() * 100) === 50){
				readings.push(true);
			}
			else{readings.push(false);}
		}
		return readings;
	}
	
	
	var postFakeCommandFromMothership = function () {
		var left = Math.ceil(Math.random() * 3);
		var right = Math.ceil(Math.random() * 3);
		var req = new XMLHttpRequest();
		req.onload = function (evt) {};
		req.withCredentials = true;
		req.open("POST", "/commands", true);
		var command = {
			pid : pid,
			left : left,
			right : right,
			duration : 0.25
		};
		req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
		req.send(JSON.stringify(command));
	}

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
	}

	var postStatusUpdate = function () {
	
		
	
		var status = {
			"pid" : pid,
			"lat" : lat + (Math.random() * 0.00001),
			"long" : lon +(Math.random() * 0.00001),
			"Vbattery" : vBattery,
			"heading": Math.random() * 360,
			"Vin": 5.0,
			"gstate": 3, // 3 = awesome, 2 = kinda ok, < 2 = dunno.
			"gtime": 10020000,
			"mtime": Date.now(),
			"beacon" : generateFakeBeaconData(),
			"obstruction" : genarateFakeObstructionData()
		};
		var req = new XMLHttpRequest();
		req.withCredentials = true;
		req.open("PUT", "/status", true);
		req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
		req.send(JSON.stringify(status));
	}

	var putTaskCompleted = function () {
		var req = new XMLHttpRequest();
		req.onload = function (evt) {
			getNextTask();
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
	}

	var simulateTaskCompletion = function () {
		setTimeout(putTaskCompleted, 1000);
	}

	return function (entityPid,location) {
		if(entityPid !== undefined){
					pid = entityPid;
					}
					if(location !== undefined){
					lat = location.latitude;
					lon = location.longitude;
					}
		
			return {				
					start : function () {
					setInterval(postFakeCommandFromMothership, 2000);
					getNextTask();
				},
				getProperties: function(){
					console.log(pid,lat,lon);
				}
				};
		}});
