define([], function () {
	
function FakeHunter(entityPid,location){
	this.lon = -76.8976622;
	this.lat = 39.1672858;	
	this.pid = 1;
	this.currentTaskID = 0;
	this.vBattery = 3.4;

	if(entityPid !== undefined){
		this.pid = entityPid;
	}
	if(location !== undefined){
			lat = location.latitude;
			lon = location.longitude;
		}
	var that = this;
	setInterval(function(){that.postFakeCommandFromMothership()}, 2000);
	this.getNextTask();
	}
	
FakeHunter.prototype.genarateFakeObstructionData = function(){
	var readings = [];
		for(var i=0;i<90;i++){
			readings.push(Math.random() * 3000);
		}
		return readings;
}

FakeHunter.prototype.generateFakeBeaconData = function(){
	var readings = [];
		for(var i = 0;i<90;i++){
			if(Math.round(Math.random() * 100) === 50){
				readings.push(true);
			}
			else{readings.push(false);}
		}
		return readings;
}
	
	FakeHunter.prototype.postFakeCommandFromMothership = function () {
		var left = Math.ceil(Math.random() * 3);
		var right = Math.ceil(Math.random() * 3);
		var req = new XMLHttpRequest();
		req.onload = function (evt) {};
		req.withCredentials = true;
		req.open("POST", "/commands", true);
		var command = {
			pid : this.pid,
			left : left,
			right : right,
			duration : 0.25
		};
		req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
		req.send(JSON.stringify(command));
	}

	FakeHunter.prototype.getNextTask = function () {
		console.log("get next task ", this.pid)
		var req = new XMLHttpRequest();
		var that = this;
		req.onload = function (evt) {
			var task = JSON.parse(req.response);
			if (task.length > 0 && task[0].cid != undefined) {
				currentTaskID = task[0].cid;
				that.simulateTaskCompletion();
				return;
			}
		
			setTimeout(function(){that.getNextTask();}, 1000);
		};
		req.withCredentials = true;
		req.open("GET", "/commands?pid=" + this.pid, true);
		req.send();
	}

	FakeHunter.prototype.postStatusUpdate = function () {
		var status = {
			"pid" : this.pid,
			"lat" : this.lat + (Math.random() * 0.00004),
			"long" : this.lon +(Math.random() * 0.00004),
			"Vbattery" : this.vBattery,
			"heading": Math.random() * 360,
			"Vin": 5.0,
			"gstate": 3, // 3 = awesome, 2 = kinda ok, < 2 = dunno.
			"gtime": 10020000,
			"mtime": Date.now(),
			"beacon" : this.generateFakeBeaconData(),
			"obstruction" : this.genarateFakeObstructionData()
		};
		var req = new XMLHttpRequest();
		req.withCredentials = true;
		req.open("PUT", "/status", true);
		req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
		req.send(JSON.stringify(status));
	}

	FakeHunter.prototype.putTaskCompleted = function () {
		var req = new XMLHttpRequest();
		var that = this;
		req.onload = function (evt) {
			that.getNextTask();
		};
		var status = {
			"cid" : this.currentTaskID,
			"pid" : this.pid,
			"complete" : true
		};
		req.withCredentials = true;
		req.open("PUT", "/commands", true);
		req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
		req.send(JSON.stringify(status));
		this.postStatusUpdate();
	}

	FakeHunter.prototype.simulateTaskCompletion = function () {
		var that = this;
		setTimeout(function(){that.putTaskCompleted();}, 1000);
	}
	
	return FakeHunter;

	});
