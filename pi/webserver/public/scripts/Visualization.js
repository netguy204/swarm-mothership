define("Visualization",["pixi/pixi"],function(PIXI){
	
	var renderer,stage,obstructionGraphics,hunterGraphics;

	var animate = function(){
		 requestAnimationFrame(animate);
		// render the container
		renderer.render(stage);
	}
	
	var drawBackground = function(){
	obstructionGraphics.lineStyle(1);
	obstructionGraphics.beginFill(0x7CB36F, 0.5);
	obstructionGraphics.drawRect(screen.availWidth/2,screen.availHeight/2,10);
	obstructionGraphics.endFill();
	}
	
	renderer = PIXI.autoDetectRenderer(800, 600,{backgroundColor : 0x1099bb});
	console.log(document);
	document.body.appendChild(renderer.view);

	// create the root of the scene graph
	stage = new PIXI.Container();
	console.log("stage",stage)
	//stage.setBackgroundColor(0x022E1E);
	stage.interactive = true;
	obstructionGraphics = new PIXI.Graphics();
	hunterGraphics = new PIXI.Graphics();
	stage.addChild(obstructionGraphics);
	stage.addChild(hunterGraphics);
	renderer.resize(window.innerWidth,window.innerHeight);
	
	// start animating
	drawBackground();
	animate();
	
	return{
		resize: function(){
			console.log(window.innerWidth,window.innerHeight);
			renderer.resize(window.innerWidth,window.innerHeight);
			animate();
		}
	}

});