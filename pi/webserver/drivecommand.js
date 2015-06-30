var target = {};

target.DriveCommand = function(existingObj) {
    var obj = existingObj || {};
    obj.properties = obj.properties || {};

    obj.properties.command = "";
    obj.properties.duration = "";
    obj.properties.target = "";
    obj.properties.executed = false;
    obj.properties.received = "";
	obj.properties.leftTread;
	obj.properties.rightTread;
	obj.properties.speed = 0;
	obj.properties.angle = 0;
	

    obj.command = function(newValue) {
        if (typeof newValue != "undefined") {
            obj.properties.command = newValue;
        }

        return obj.properties.command;
    };

    obj.duration = function(newValue) {
        if (typeof newValue != "undefined") {
            obj.properties.duration = newValue;
        }

        return obj.properties.duration;
    };

    obj.executed = function(newValue) {
        if (typeof newValue == "boolean") {
            obj.properties.executed = newValue;
        }

        return obj.properties.executed;
    };

    obj.target = function(newValue) {
        if (typeof newValue != "undefined") {
            obj.properties.target = newValue;
        }

        return obj.properties.target;
    };

    obj.received = function(newValue) {
        if (typeof newValue == "number") {
            obj.properties.received = newValue;
        }

        return obj.properties.received;
    };

    obj.toJson = function() {
        return JSON.stringify(obj.properties);
    };

    obj.fromJson = function(source) {
        var thing = JSON.parse(source);
        obj.patch(thing);
    };

    obj.patch = function(source) {
        for(var propertyId in source) {
            if (typeof obj[propertyId] == "function") {
                obj[propertyId](source[propertyId]);
            }
        }
    };

    return obj;
};

module.exports = target;