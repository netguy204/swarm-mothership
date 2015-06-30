var connect = require('connect');
var serveStatic = require("serve-static");
var http = require('http');
var bodyParser = require('body-parser');
var member = require('./member');
var drive = require('./drivecommand');

var app = connect();

var members = [];
var driveCommands = [];
var lastDriveCommand = Date.now();

function updateMember(id, data) {
console.log("update member");
    for (var memId in members) {
        if (members[memId].id() == id) {
            members[memId].patch(data);
            return members[memId];
        }
    }
    return false;
}

function addDriveCommand(command) {
    lastDriveCommand = Date.now();
    command.received(lastDriveCommand);
    driveCommands.push(command);
}

app.use(bodyParser.json());
console.log("Hosting " + __dirname + "/public statically");
app.use(serveStatic(__dirname + "/public/"));

app.use("/status", function(req, res, next) {
/*
status:
[lat,long,pid,heading,obstruction,beacon detection]
array of floats
*/
    res.setHeader('Content-Type', 'application/json');
    var propertiesList = [];

    for (var idx in members) {
        propertiesList.push(members[idx].properties);
    }
    res.end(JSON.stringify(propertiesList));
    next();
});


app.use("/drive", function(req, res, next) {
    res.setHeader('Content-Type', 'application/json');

    var command = req.body;

    if (typeof command.command != "undefined") {
        var cmd = drive.DriveCommand();
        cmd.patch(command);
        addDriveCommand(cmd);
    }

    var propertiesList = [];
    for (var idx in driveCommands) {
        propertiesList.push(driveCommands[idx].properties);
    }
    res.end(JSON.stringify(propertiesList));

    next();
});


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


app.use(function(req, res, next) {
    res.end('Current pages are: status, update, drive');
});



console.log("started");
http.createServer(app).listen(8080);
