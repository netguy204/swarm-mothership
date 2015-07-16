  var autodrive = {};

  autodrive.mover = function(existingObj) {
    var obj = existingObj || {};
    obj.properties = obj.properties || {};

    obj.properties.name = "";
    obj.properties.id = "";
    obj.properties.latitude = "";
    obj.properties.longitude = "";
    obj.properties.heading = "";
    obj.properties.turn = "";
    obj.properties.turnMax = "";
    obj.properties.turnMin = "";
    obj.properties.turnStep = "";
    obj.properties.speed = "";
    obj.properties.speedMax = "";
    obj.properties.speedMin = "";

    obj.name = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.name = newValue;
      }

      return obj.properties.name;
    };

    obj.id = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.id = newValue;
      }

      return obj.properties.id;
    };

    obj.latitude = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.latitude = newValue;
      }

      return obj.properties.latitude;
    };

    obj.longitude = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.longitude = newValue;
      }

      return obj.properties.longitude;
    };

    obj.heading = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.heading = newValue;
      }

      return obj.properties.heading;
    };

    obj.turn = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.turn = newValue;
      }

      return obj.properties.turn;
    };

    obj.turnMax = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.turnMax = newValue;
      }

      return obj.properties.turnMax;
    };

    obj.turnMin = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.turnMin = newValue;
      }

      return obj.properties.turnMin;
    };

    obj.turnStep = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.turnStep = newValue;
      }

      return obj.properties.turnStep;
    };

    obj.speed = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.speed = newValue;
      }

      return obj.properties.speed;
    };

    obj.speedMax = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.speedMax = newValue;
      }

      return obj.properties.speedMax;
    };

    obj.speedMin = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.speedMin = newValue;
      }

      return obj.properties.speedMin;
    };

    obj.getLocation = function() {
	var result = { latitude: obj.latitude(), longitude: obj.longitude()};
	return result;
    };

    return obj;
  };

  autodrive.target = function(existingObj) {
    var obj = existingObj || {};
    obj.properties = obj.properties || {};

    obj.properties.name = "";
    obj.properties.id = "";
    obj.properties.latitude = "";
    obj.properties.longitude = "";

    obj.name = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.name = newValue;
      }

      return obj.properties.name;
    };

    obj.id = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.id = newValue;
      }

      return obj.properties.id;
    };

    obj.latitude = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.latitude = newValue;
      }

      return obj.properties.latitude;
    };

    obj.longitude = function(newValue) {
      if (typeof newValue != "undefined") {
        obj.properties.longitude = newValue;
      }

      return obj.properties.longitude;
    };

    obj.getLocation = function() {
	var result = { latitude: obj.latitude(), longitude: obj.longitude()};
	return result;
    };

    return obj;
  };

module.exports = { mover : autodrive.mover, target : autodrive.target };
