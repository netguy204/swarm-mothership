define("ControlInput",[],function(){
	
	var interval;
	var heading = 0;
	var speed = 0;
	var entityPID = 0;
	
	var arrowsDown = {
		up: false,
		down: false,
		left: false,
		right: false
	}
	

	var modifyCommand = function(keyEvt,state){
		switch(keyEvt.keyCode){
			//left
			case 37:
				decrementHeading();
				arrowsDown.left = state;
				break;
			//up
			case 38:
				speed = 255;
				arrowsDown.up = state;
				break;
			//right
			case 39:
				incrementHeading();
				arrowsDown.right = state;
				break;
			//down
			case 40:
				speed = -255;
				arrowsDown.down = state;
				break;
		}
	};
	
	var incrementHeading = function(){
		heading++;
		if(heading>360){heading = heading % 360;}
		console.log(heading);
	};
	
	var decrementHeading = function(){
		heading--;
		if(heading<0){heading += 360;}
		console.log(heading);
	};
	
	var sendCommand = function(){
	if(arrowsDown.up || arrowsDown.down || arrowsDown.left || arrowsDown.right){
		var req = new XMLHttpRequest();
		req.onload = function(response){};
		req.withCredentials = true;
		req.open("POST", "/commands", true);
		req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
	    command = {
			pid: entityPID,
			type: "DRIVE",
			heading: heading,
			speed: speed,
			duration: 0.25
		};
		console.log(command);
		req.send(JSON.stringify(command));
		}
	};

	document.body.onkeydown = function(evt){
		modifyCommand(evt,true);
	};
	document.body.onkeyup = function(evt){
		modifyCommand(evt,false);
	};
	
	interval = setInterval(sendCommand,250);

	
	return function(){
		return{
			"setHeading": function(resetValue){heading = resetValue;},
			"setEntityPID": function(pid){entityPID = pid;}
		};
	};
});