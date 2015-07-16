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

var app = connect();

var platforms = [];
var commands = [];

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

app.use(bodyParser.json());
app.use(function (req, res, next) {
	req.query = querystring.parse(req._parsedUrl.query);
	next();
});

app.use(serveStatic(__dirname + "/public/"));

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

/*
app.use("/update", function(req, res, next) {
res.setHeader('Content-Type', 'application/json');

var thing = req.body;
var result = updateMember(thing.id, thing);

if (result != false) {
res.end(JSON.stringify(result));
}
else {
if (typeof thing.id != "undefined") {
var item = member.Member();
item.patch(req.body);
members.push(item);
res.end(item.toJson());
}
else {
res.end(JSON.stringify({error: "no id"}));
}
}

next();
});
 */

app.use(function (req, res, next) {
	res.end('Current pages are: status, commands');
});

console.log("started");
http.createServer(app).listen(8080);
