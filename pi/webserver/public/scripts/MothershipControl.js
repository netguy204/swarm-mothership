define("MothershipControl",[],function(){
	
	var interval;
	
	var arrowsDown = {
		up: false,
		down: false,
		left: false,
		right: false
	}

	var modifyCommand = function(keyEvt,state){
		switch(keyEvt.keyCode){
			case 37:
				arrowsDown.left = state;
				break;
			case 38:
				arrowsDown.up = state;
				break;
			case 39:
				arrowsDown.right = state;
				break;
			case 40:
				arrowsDown.down = state;
				break;
		}
	};
	
	var sendCommand = function(){
	if(arrowsDown.up || arrowsDown.down || arrowsDown.left || arrowsDown.right){
		var req = new XMLHttpRequest();
		req.onload = function(response){updateHunterPositions(response)};
		req.withCredentials = true;
		req.open("POST", "http://localhost:8080/commands", true);
		req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
	    command = {
			pid: 0,
			left: arrowsDown.left,
			right:arrowsDown.right,
			up:arrowsDown.up,
			down:arrowsDown.down,
			duration: 0.25
		};
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

	
	return{
	};
});