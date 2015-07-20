define("StatusBox",[],function(){
	var hidden = false;
	var outer = document.createElement("div");
	outer.className += "cesium-infoBox cesium-infoBox-visible";
	outer.style.width = "400px";
	outer.style.height = "400px";
	document.body.appendChild(outer);
	
	return{
		updateStatus: function(statuses){
			outer.innerHTML = "";
			if(statuses.length < 1){
				this.hide();
				return;
			}
			else{
				if(hidden === true){this.show();}
			}
			for(var idx in statuses){
				var entry = document.createElement("div");
				var s = statuses[idx];
				entry.innerHTML = s.pid;
				
				if(s.pid === 0){
					entry.innerHTML += " - Mothership";
				}
				
				if(s.Vbattery !== undefined){
					entry.innerHTML += " - Battery: " + s.Vbattery;
				}
				if(s.mtime !== undefined){
					entry.innerHTML += " -  Last Update: " + s.mtime;
				}
				outer.appendChild(entry);
			}
		},
		
		show: function(){
			outer.style.display = "block";
			hidden = false;
		},
		
		hide: function(){
			outer.style.display = "none";
			hidden = true;
		}
	};
});