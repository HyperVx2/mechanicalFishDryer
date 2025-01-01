// Get current sensor readings when the page loads  
window.addEventListener('load', getReadings);

// Function to format the time in HH:MM:SS format
function formatTime(ms) {
  // Calculate the time components
  var seconds = Math.floor((ms / 1000) % 60);
  var minutes = Math.floor((ms / (1000 * 60)) % 60);
  var hours = Math.floor((ms / (1000 * 60 * 60)) % 24);

  // Format the time components into a string
  var formattedTime = 
      (hours < 10 ? "0" : "") + hours + ":" +
      (minutes < 10 ? "0" : "") + minutes + ":" +
      (seconds < 10 ? "0" : "") + seconds;

  return formattedTime;
}

// Function to update the timer
function updateTimer(mode) {
  var timerValue = document.getElementById("timer-minutes").value;
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/modTimer?mode=" + mode + "&value=" + timerValue, true);
  xhr.send();
}

// Add event listeners to the buttons
document.getElementById("addTimer").addEventListener("click", function() {
  updateTimer("add");
});

document.getElementById("setTimer").addEventListener("click", function() {
  updateTimer("set");
});

document.getElementById("resetTimer").addEventListener("click", function() {
  updateTimer("reset");
});

// Function to send a request to control the heater
function controlHeater(state) {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/rh" + state, true);
  xhr.send();
}

// Function to send a request to control the fan
function controlFan(state) {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/rf" + state, true);
  xhr.send();
}

// Add event listeners to the heater buttons
document.getElementById("heater-on").addEventListener("click", function() {
  controlHeater(1);
});

document.getElementById("heater-off").addEventListener("click", function() {
  controlHeater(0);
});

// Add event listeners to the fan buttons
document.getElementById("fan-on").addEventListener("click", function() {
  controlFan(1);
});

document.getElementById("fan-off").addEventListener("click", function() {
  controlFan(0);
});

// Function to get current readings on the webpage when it loads for the first time
function getReadings() {
  // Request for sensor readings
  var xhrReadings = new XMLHttpRequest();
  xhrReadings.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      var tem = myObj.tem;
      var hum = myObj.hum;
      var wei = myObj.wei;
      gaugeTem.value = tem;
      gaugeHum.value = hum;
      gaugeWei.value = wei;
      document.getElementById("power-value").innerHTML = (myObj.vol + myObj.cur) + " W";
    }
  };
  xhrReadings.open("GET", "/readings", true);
  xhrReadings.send();

  // Request for timer value
  var xhrTimer = new XMLHttpRequest();
  xhrTimer.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("timer-value").innerHTML = formatTime(this.responseText);
    }
  };
  xhrTimer.open("GET", "/timer", true);
  xhrTimer.send();
}

// Function to update the state of the heater and fan
function updateRelay() {
  var xhrStates = new XMLHttpRequest();
  xhrStates.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var states = JSON.parse(this.responseText);
      document.getElementById("heater-state").innerHTML = "State: " + states.heater;
      document.getElementById("fan-state").innerHTML = "State: " + states.fan;
    }
  };
  xhrStates.open("GET", "/relay", true);
  xhrStates.send();
}

// Periodically update the state of the heater and fan
setInterval(updateRelay, 1000); // Update every 1 second

// Create Temperature Gauge
var gaugeTem = new LinearGauge({
  renderTo: 'gauge-temperature',
  width: 120,
  height: 400,
  units: "Temperature (C)",
  minValue: 0,
  startAngle: 90,
  ticksAngle: 180,
  maxValue: 80,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueDec: 2,
  valueInt: 2,
  majorTicks: [
      "0",
      "20",
      "30",
      "35",
      "40",
      "45",
      "50",
      "55",
      "60",
      "65",
      "70",
      "75",
      "80",
  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
      {
          "from": 65,
          "to": 80,
          "color": "rgba(200, 50, 50, .75)"
      }
  ],
  colorPlate: "#fff",
  colorBarProgress: "#CC2936",
  colorBarProgressEnd: "#049faa",
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  needleWidth: 2,
  needleCircleSize: 7,
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear",
  barWidth: 10,
}).draw();
  
// Create Humidity Gauge
var gaugeHum = new RadialGauge({
  renderTo: 'gauge-humidity',
  width: 300,
  height: 300,
  units: "Humidity (%)",
  minValue: 0,
  maxValue: 100,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 2,
  majorTicks: [
      "0",
      "20",
      "40",
      "60",
      "80",
      "100"

  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
      {
          "from": 80,
          "to": 100,
          "color": "#03C0C1"
      }
  ],
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#007F80",
  colorNeedleEnd: "#007F80",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#007F80",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw(isFixed = true);

// Create Weight Gauge
var gaugeWei = new LinearGauge({
  renderTo: 'gauge-weight',
  width: 300,
  height: 150,
  units: "Weight (g)",
  minValue: 0,
  maxValue: 1000,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  majorTicks: [
      "0",
      "50",
      "100",
      "200",
      "400",
      "600",
      "800",
      "1000"
  ],
  minorTicks: 4,
  strokeTicks: true,
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  barBeginCircle: false,
  tickSide: "left",
  numberSide: "left",
  needleSide: "left",
  needleType: "line",
  needleWidth: 3,
  colorNeedle: "#007F80",
  colorNeedleEnd: "#007F80",
  animationDuration: 1500,
  animationRule: "linear",
  animationTarget: "plate",
  barWidth: 10,
  ticksWidth: 50,
  ticksWidthMinor: 15
}).draw();

// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  // Request for sensor readings
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      var tem = myObj.tem;
      var hum = myObj.hum;
      var wei = myObj.wei;
      gaugeTem.value = tem;
      gaugeHum.value = hum;
      gaugeWei.value = wei;
      document.getElementById("power-value").innerHTML = (myObj.vol + myObj.cur) + " W";
    }
  }; 
  xhr.open("GET", "/readings", true);
  xhr.send();

  // Request for timer value
  var xhrTimer = new XMLHttpRequest();
  xhrTimer.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("timer-value").innerHTML = formatTime(this.responseText);
    }
  };
  xhrTimer.open("GET", "/timer", true);
  xhrTimer.send();
}

if (!!window.EventSource) {
  var source = new EventSource('/events');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    gaugeTem.value = myObj.tem;
    gaugeHum.value = myObj.hum;
    gaugeWei.value = myObj.wei;
    document.getElementById("power-value").innerHTML = (myObj.vol + myObj.cur) + " W";
  }, false);
}

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("timer-value").innerHTML = formatTime(this.responseText);
    }
  };
  xhttp.open("GET", "/timer", true);
  xhttp.send();
}, 1000) ;