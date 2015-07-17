define("MothershipControl", [], function () {

	var interval;
	var servoAngle = 0; //-30 to 30
	var speed = 0; //-60 to 60
	var entityPID = 0;

	var arrowsDown = {
		up : false,
		down : false,
		left : false,
		right : false
	}

	var modifyCommand = function (keyEvt, state) {
		switch (keyEvt.keyCode) {
			//left
		case 37:
			decrementAngle();
			arrowsDown.left = state;
			break;
			//up
		case 38:
			incrementSpeed();
			arrowsDown.up = state;
			break;
			//right
		case 39:
			incrementAngle();
			arrowsDown.right = state;
			break;
			//down
		case 40:
			decrementSpeed();
			arrowsDown.down = state;
			break;
		}
	};

	var incrementAngle = function () {
		if(servoAngle === 30){return;}
		servoAngle++;
	};

	var decrementAngle = function () {
		if(servoAngle === -30){return;}
		servoAngle--;
	};
	
	var incrementSpeed = function(){
		if(speed === 60){return;}
		speed++;
	}
	
	var decrementSpeed = function(){
		if(speed === -60){return}{
			speed--;
		}
	}

	var sendCommand = function () {
		if (arrowsDown.up || arrowsDown.down || arrowsDown.left || arrowsDown.right) {
			var req = new XMLHttpRequest();
			req.onload = function (response) {};
			req.withCredentials = true;
			req.open("POST", "/commands", true);
			req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
			command = {
				pid : entityPID,
				type : "DRIVE",
				angle : servoAngle,
				speed : speed,
				duration : 0.25
			};
			console.log(command);
			req.send(JSON.stringify(command));
			return;
		}
		speed = Math.ceil(speed/2);
		servoAngle = Math.ceil(servoAngle/2);
	};

	document.body.onkeydown = function (evt) {
		modifyCommand(evt, true);
	};
	document.body.onkeyup = function (evt) {
		modifyCommand(evt, false);
	};

	interval = setInterval(sendCommand, 250);

	return function () {
		return {
			"setHeading" : function (resetValue) {
				heading = resetValue;
			},
			"setEntityPID" : function (pid) {
				entityPID = pid;
			}
		};
	};
});
