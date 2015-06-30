var target = {};

target.Member = function(existingObj) {
    var obj = existingObj || {};
    obj.properties = obj.properties || {};

    obj.properties.id = "";
    obj.properties.lat = "";
    obj.properties.long = "";
    obj.properties.orientation = "";
    obj.properties.lastUpdated = Date.now();
    obj.properties.speed = "";
    obj.properties.type = "";

    obj.id = function(newValue) {
        if (typeof newValue != "undefined") {
            obj.properties.id = newValue;
        }

        return obj.properties.id;
    };

    obj.lat = function(newValue) {
        if (typeof newValue != "undefined") {
            obj.properties.lat = newValue;
        }

        return obj.properties.lat;
    };

    obj.long = function(newValue) {
        if (typeof newValue != "undefined") {
            obj.properties.long = newValue;
        }

        return obj.properties.long;
    };

    obj.orientation = function(newValue) {
        if (typeof newValue != "undefined") {
            obj.properties.orientation = newValue;
        }

        return obj.properties.orientation;
    };

    obj.lastUpdated = function(newValue) {
        if (typeof newValue != "undefined") {
            obj.properties.lastUpdated = newValue;
        }

        return obj.properties.lastUpdated;
    };

    obj.speed = function(newValue) {
        if (typeof newValue != "undefined") {
            obj.properties.speed = newValue;
        }

        return obj.properties.speed;
    };

    obj.type = function(newValue) {
        if (typeof newValue != "undefined") {
            obj.properties.type = newValue;
        }

        return obj.properties.type;
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