
var ipAddress = "172.56.36.144"; // Hard coded IP address
var port = "3001"; // Same port specified as argument to server


// for temperature
var commands = ["current", "average", "high", "low"];
var commandstyle = ["Celsius", "Fahrenheit"];
var commandtype = 0;
var csbool = false;
var modebool = false;

// for weather
var temp_c = "No_Data";


Pebble.addEventListener("appmessage",
  function(e) {
    if (e.payload) {
      if (e.payload.req_msg) {
        // ask for temperature data, including average, highest, lowest, current
        if (e.payload.req_msg == 'temperature') {
          Pebble.sendAppMessage({ "0": "Retrieving data" });
          sendToServerTemp(commands[commandtype]);
          commandtype ++;
          if (commandtype == 4) {
            commandtype = 0;
          }
        // ask server to change temperature unit
        } else if (e.payload.req_msg == 'style') {
          Pebble.sendAppMessage({ "0": "Changing unit" });
          if (csbool) {
            sendToServerTemp("Celsius");
          } else {
            sendToServerTemp("Fahrenheit");
          }
          csbool = !csbool;
        // ask server to turn on or turn off stand-by mode
        } else if (e.payload.req_msg == 'mode') {
          if (modebool) {
            Pebble.sendAppMessage({ "0": "stand-by mode off" });
            sendToServerTemp("resume");
          } else {
            Pebble.sendAppMessage({ "0": "stand-by mode on" });
            sendToServerTemp("stop");
          }
          modebool = !modebool;
        // ask for movie information
        } else if (e.payload.req_msg == 'movie') {
          sendToServerMovie();
        } else if (e.payload.req_msg == 'distance') {
          sendToServerTemp(e.payload.req_msg);
        } else if (e.payload.req_msg == 'weather') {
          sendToServerWeather();
        }
        else if (e.payload.req_msg == 'weather_sendback'){
          sendToServerTemp('weather_sendback/'+temp_c);
        } else {
          sendToServerTemp(e.payload.req_msg);
          //Pebble.sendAppMessage({ "0": "invalid" });
        }
      } else Pebble.sendAppMessage({ "0": "nokey" });
    } else Pebble.sendAppMessage({ "0": "nopayload" });
  }
);


// temperature server
function sendToServerTemp(request) {
  var req = new XMLHttpRequest();
//   var ipAddress = "158.130.213.233"; // Hard coded IP address
//   var port = "3001"; // Same port specified as argument to server
  var url = "http://" + ipAddress + ":" + port + "/";
  var method = "GET";
  var async = true;
  
  req.onload = function(e) {
    // see what came back
    if (request[0] != 'w') {
      var msg = "no response";
      var response = JSON.parse(req.responseText);
      if (response) {
        if (response.name) {
          msg = response.name; // key is name, return value of key
        } else msg = "noname";
      }
      // sends message back to pebble
      Pebble.sendAppMessage({ "0": msg });
    }
  };
  
  // error handler
  req.onerror = function() {
    if (request[0] != 'w') {
     Pebble.sendAppMessage({"0": "Couldn't connect to server."});
    }
  }
  
  req.open(method, url + request, async);
  req.send(null);
}

    
    
    
// movie list server
function sendNextItem(items, index) {
  // Build message
  var key = index;
  var dict = {};
  if (index == 0) {
    dict[key] = items.length;
  } else {
    dict[key] = items[index-1];
  }

  // Send the message
  Pebble.sendAppMessage(dict, function() {
    // Use success callback to increment index
    index++;

    if(index <= items.length) {
      // Send next item
      sendNextItem(items, index);
    } else {
      console.log('Last item sent!');
    }
  }, function() {
    console.log('Item transmission failed at index: ' + index);
  });
}

function sendList(items) {
  var index = 0;
  sendNextItem(items, index);
}


function sendToServerMovie() {
  var req = new XMLHttpRequest();
  var url = "http://www.cinemark.com/mobiletheatreshowtimes.aspx?node_id=83876";
  var method = "GET";
  var async = true;
  
  req.onload = function(e) {
    // log
    console.log('Got response: ' + this.responseText);
    // see what came back
    var msg = req.responseText.split("title=\">");
    var i;
    var movies = [];
    for (i = 1; i < msg.length; i++) {
      var movie = msg[i].split("<")[0];
      movies.push(movie);
    }
    sendList(movies);
  };
  req.open(method, url, async);
  req.send(null);
}

// motion server
function sendToServerMotion() {
  
}

// weather server
function sendToServerWeather() {
  var apiId = '94a5d903355cd511';
  var st = 'PA';
  var city = 'Philadelphia';
  
  var req = new XMLHttpRequest();
  var url = 'http://api.wunderground.com/api/'+apiId+'/conditions/q/'+st+'/'+city+'.json';
  var method = "GET";
  var async = true;
  req.onload = function(e) {
    // see what came back
    var weather_msg = [];
    var msg = "";
    var nodata = "No Data";
    var response = JSON.parse(req.responseText);
    if (response) {
      if (response.current_observation) {
        if (response.current_observation.temp_c) {
          temp_c = response.current_observation.temp_c;
        } else {
          temp_c = "No_Data";
        }
        
        if (response.current_observation.display_location.city) {
          msg = "City: " + response.current_observation.display_location.city;
        } else {
          msg = "City: " + nodata;
        }
        weather_msg.push(msg);
        if (response.current_observation.weather) {
          msg = "Weather: " + response.current_observation.weather;
        } else {
          msg = "Weather: " + nodata;
        }
        weather_msg.push(msg);
        if (response.current_observation.wind_string) {
          msg = "Wind: " + response.current_observation.wind_string;
        } else {
          msg = "Wind: " + nodata;
        }
        weather_msg.push(msg);
        if (response.current_observation.temperature_string) {
          msg = "Temp: " + response.current_observation.temperature_string;
        } else {
          msg = "Temp: " + nodata;
        }
        weather_msg.push(msg);
        if (response.current_observation.feelslike_string) {
          msg = "Feels like: " + response.current_observation.feelslike_string;
        } else {
          msg = "Feels like: " + nodata; 
        }
        weather_msg.push(msg); 
      } else msg = "No Weather Info"; 
    }
    // sends message back to pebble  
    sendList(weather_msg);
  };
  
  // Error handler: e.g. when server is not responsing..
  req.onerror = function () {
    Pebble.sendAppMessage({
      "0": "Couldn't connect to server."
    });
  };
  req.open(method, url, async);
  req.send(null);
}