define("StatusBox",[],function(){
	var outer = document.createElement("div");
	outer.className += "cesium-infoBox cesium-infoBox-visible";
	outer.style.width = "300px";
	outer.style.height = "400px";
	
	document.body.appendChild(outer);
	
	
	
	return{
		updateStatus: function(statuses){
			outer.innerHTML = "";
			for(var idx in statuses){
				var s = statuses[idx];
				outer.innerHTML +=s.pid + " - Battery: " + s.Vbattery + " - Last Update: " + s.mtime + "\n"; 
			}
		},
		
		show: function(){
			outer.style.display = "block";
		},
		
		hide: function(){
			outer.style.display = "none";
		}
	};
});