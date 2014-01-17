var util = require('util');
var SerialPort = require('serialport').SerialPort;
var xbee_api = require('./lib/xbee-api.js');

var C = xbee_api.constants;

var xbeeAPI = new xbee_api.XBeeAPI({
  api_mode: 1
});

var serialport = new SerialPort("/dev/ttyUSB0", {
  baudrate: 9600,
  parser: xbeeAPI.rawParser()
});

//ORDER_CODE
var SYNC=0x10;
var DELAY_RESP = 0x14;
var GET_STATUS = 0x20;
var SET_STATUS = 0x22;
var DRILL_READY = 0x30;
var DRILL_RESET = 0x31;

//RESPONSE_CODE
var RESP = 0x12;
var DELAY_REQ = 0x13;
var DELAY_FEEDBACK = 0x15;
var MEASURE_START = 0x41;
var MEASURE_END = 0x42;
var MEASURE_OK = 0x44;
var MEASURE_READYD = 0x45;
var MEASURE_RESETTED = 0x46;
var STATUS = 0x21;




serialport.on("open", function() {
  console.log("Serial port open... sending ATND");
  var frame_RESET = {
    type: 0x01,
    id: 0x01,
    destination16: "FFFF",
    options: 0x01,
    data: [DRILL_RESET]
  };

  var frame_SYNC = {
    type: 0x01,
    id: 0x01,
    destination16: "FFFF",
    options: 0x01,
    data: [ SYNC ]
  };

  var frame_DELAY_RESP = {
    type: 0x01,
    id: 0x01,
    destination16: "FFFF",
    options: 0x01,
    data: [ DELAY_RESP ]
  };

  var frame_TEST_DRILL = {
    type: 0x01,
    id: 0x01,
    destination16:"FFFF",
    options: 0x01,
    data: [ DRILL_READY, 0x40, 0x02, 0x02, 0x02 ]
  };

  serialport.write(xbeeAPI.buildFrame(frame_RESET), function(err, res) {
  if (err) throw(err);
  else console.log ("reset");
  });

  setTimeout(function () {
    serialport.write(xbeeAPI.buildFrame(frame_SYNC), function(err, res) {
    if (err) throw(err);
    else     console.log("sync: "+util.inspect(res));
  });
}, 1000);
  setTimeout(function () {
  serialport.write(xbeeAPI.buildFrame(frame_DELAY_RESP), function(err, res) {
    if (err) throw(err);
    else     console.log("DELAY_RESP: "+util.inspect(res));
  });

  }, 4000);
  setTimeout(function(){
	  serialport.write(xbeeAPI.buildFrame(frame_TEST_DRILL), function(err, res) {
    if (err) throw(err);
    else     console.log("test drill:"+util.inspect(res));
  });

  }, 5000);

  
//  serialport.write(xbeeAPI.buildFrame(frame), function(err, res) {
//    if (err) throw(err);
 //   else     console.log("written bytes: "+util.inspect(res));
//  });
});


xbeeAPI.on("frame_object", function(frame) {
var TX16 = 0x81;
  console.log("OBJ> "+util.inspect(frame));
  if(frame.type == TX16){
    console.log("Remote16 : "+ frame.remote16 + "  Response : 0x"+Number(frame.data[0]).toString(16));
  }
});



