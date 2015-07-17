/**
 *
 * platforms = keyed on pid. All of the things participating in the
 * swarm.
 *
 * A few common properties:
 * cid = command id
 * pid = platform id
 * lat/long = position in geodetic coordinates

 * ecef_pos = position in earth centered earth fixed

 * ecef_vel = velocity

 * enu_pos = position in east/north/up. This origin of this coordinate
 * system is often platform defined but this is useful for relative
 * measurements
 */

var connect = require('connect');
var serveStatic = require("serve-static");
var http = require('http');
var bodyParser = require('body-parser');
var querystring = require('querystring');
var autodrive = require('./autodrive');

var app = connect();

var platforms = [];
var commands = [];
var mothership = autodrive.mover();

var nextCommandId = 0;

function merge(o1, o2) {
	var o3 = {};
	for (var attr in o1) {
		o3[attr] = o1[attr];
	}
	for (attr in o2) {
		o3[attr] = o2[attr];
	}
	return o3;
}

function findPlatform(pid) {
	for (var idx in platforms) {
	    if (platforms[idx].integrated.pid == pid) return platforms[idx];
	}
	console.log("not found");
};

app.use(bodyParser.json());
app.use(function (req, res, next) {
	req.query = querystring.parse(req._parsedUrl.query);
	next();
});

app.use(serveStatic(__dirname + "/public/"));

/* Given a lat/long, which direction, and how far to go to a destination? */
// navigate?from=0&to=1 -- Navigate from platform 0 to platform 1
// navigate?from=0&to={"lat":39.2833,%20"long":-76.6157} -- navigate from platform 0 to a lat/long
app.use("/navigate", function(req, res, next) {
	res.setHeader('Content-Type', 'application/json');

	var from = req.query.from || undefined;
	var to = req.query.to || undefined;

	// Get mothership's position
	var platformData = findPlatform(from);
	mothership.updateStatus(platformData.integrated.lat, platformData.integrated.long, platformData.integrated.heading);

	// Get ID or object for target
	var dest = autodrive.target();
	if (to.indexOf("{") === 0) {
		// In case it's an object, and not an ID
		to = JSON.parse(to);
		dest.latitude(to.lat);
		dest.longitude(to.long);
	}
	else {
		var target = findPlatform(to);
		dest.latitude(target.integrated.lat);
		dest.longitude(target.integrated.long);
	}

	var directions = mothership.getDriveInstructions(dest);

	// TODO
	// If the mothership's queue is EMPTY, and directions.action == "drive"
	// use directions.turn {left/right/straight} to determine what actions to add to the queue

	res.end(JSON.stringify(directions));
	next();
});

app.use("/status", function (req, res, next) {
	/*
	status:
	[lat,long,pid,heading,obstruction,beacon detection]
	array of floats
	 */
	res.setHeader('Content-Type', 'application/json');
	if (req.method == "GET") {
		res.end(JSON.stringify(platforms.map(function (p) {
					return p.integrated;
				})));
		next();
	} else if (req.method == "PUT") {
		var update = req.body;
		console.log(update);
		var found = false;
		for (var idx in platforms) {
			if (platforms[idx].integrated.pid == update.pid) {
				platforms[idx].integrated = merge(platforms[idx].integrated, update);
				platforms[idx].history.push(update);
				found = true;
				break;
			}
		}
		if (!found) {
			platforms.push({
				integrated : update,
				history : [update]
			});
		}
		res.end(JSON.stringify({
				success : true
			}));
		next();
	}
});

app.use("/history", function (req, res, next) {
	res.setHeader('Content-Type', 'application/json');

	var pid = req.query.pid || undefined;
	var result = [];
	for (var idx in platforms) {
		if (platforms[idx].integrated.pid == pid) {
			result = platforms[idx].history;
			break;
		}
	}
	res.end(JSON.stringify(result));
	next();
});

app.use("/killQueue", function (req, res, next) {
	commands = [];
	next();
});

app.use("/scanResults", function (req, res, next) {
	if(req.method === "PUT"){
				var update = req.body;
		console.log(update);
		var found = false;
		for (var idx in platforms) {
			if (platforms[idx].integrated.pid == update.pid) {
				platforms[idx].integrated = merge(platforms[idx].integrated, update);
				platforms[idx].history.push(update);
				found = true;
				break;
			}
		}
		if (!found) {
			platforms.push({
				integrated : update,
				history : [update]
			});
		}
		res.end(JSON.stringify({
				success : true
			}));
		next();
	}
});

app.use("/commands", function (req, res, next) {
	res.setHeader('Content-Type', 'application/json');
	var command = req.body;
	if (req.method == "GET") {
		// GET the next thing out of the command queue
		var pid = req.query.pid;
		var max = req.query.max || 1;
		var all = req.query.all || false;

		var result = commands;
		if (!all) {
			result = result.filter(function (c) {
					return c.complete != true;
				});
		}

		if (pid !== undefined) {
			result = result.filter(function (c) {
					return c.pid == pid;
				});
		}

		if (max) {
			result = result.slice(0, max);
		}

		console.log(result);

		res.end(JSON.stringify(result));
		next();
	} else if (req.method == "POST") {
		// POST a new thing to the command queue
		console.log("POST");
		command.cid = nextCommandId;
		nextCommandId++;
		commands.push(command);
		console.log(command);
		res.end(JSON.stringify({
				success : true
			}));
		next();
	} else if (req.method == "PUT") {
		// PUT an update into a particular slot in the command queue
		var cid = command.cid;
		if (cid === undefined) {
			console.log("PUT command with no CID");
			res.end(JSON.stringify({
					success : false,
					message : "PUT command with no CID"
				}));
			next();
		} else {
			console.log("setting task " + cid + " to complete");
			for (var idx in commands) {
				if (commands[idx].cid == cid) {
					commands[idx].complete = command.complete;
					res.end(JSON.stringify({
							success : true
						}));
					console.log("PUT SUCCESS");
					console.log(command);
					return next();
				}
			}
			res.end(JSON.stringify({
					success : false,
					message : "PUT couldnt find CID"
				}));
			return next();
		}
	}
	return undefined;
});


app.use(function (req, res, next) {
	res.end('Current pages are: status, commands, history, killQueue, navigate');
});

console.log("started");
http.createServer(app).listen(8080);
