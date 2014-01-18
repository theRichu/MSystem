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
var DELAY_REQ = 0x18;
var DELAY_FEEDBACK = 0x15;
var MEASURE_START = 0x41;
var MEASURE_END = 0x42;
var MEASURE_OK = 0x49;
var MEASURE_READYD = 0x45;
var MEASURE_RESETTED = 0x46;
var STATUS = 0x21;

var t1;
var t2;
var t4;
var t6;
var t7;
var t9;
var t10;


  var frame_DELAY_RESP = {
    type: 0x01,
    id: 0x01,
    destination16: "FFFF",
    options: 0x01,
    data: [ DELAY_RESP ]
  };


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


//Sync
  setTimeout(function () {
    t1=new Date().getTime();
    serialport.write(xbeeAPI.buildFrame(frame_SYNC), function(err, res) {
    if (err) throw(err);
    else     console.log("sync: "+util.inspect(res));
  });
}, 1000);
     
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
    var time_temp = new Date().getTime();

    if(frame.data[0]==RESP){
      t4 = time_temp;
      console.log("t4 : " + t4);
    }else if(frame.data[0]==DELAY_REQ){
        t7 = new Date().getTime();
        console.log("t7 : " + t7);
        serialport.write(xbeeAPI.buildFrame(frame_DELAY_RESP), function(err, res) {
          if (err) throw(err);
          else  console.log("DELAY_RESP: "+util.inspect(res));
        });


      t6 = time_temp;
      console.log("t6 : " + t6);
  
      t2 = Number(frame.data[1])+ Number(Number(frame.data[2])<<8) + Number(Number(frame.data[3])<<16) + Number(Number(frame.data[4])<<24);
      
      console.log("t2 : " + t2);
    }else if(frame.data[0]==DELAY_FEEDBACK){
      t10 = time_temp;
      console.log("t10 : " + t10);
      t9 = Number(frame.data[1])+ Number(Number(frame.data[2])<<8) + Number(Number(frame.data[3])<<16) + Number(Number(frame.data[4])<<24);
      console.log("t9 : " + t9);
      //Time Sync Complete

      console.log("delay(m>s) : " + (t2-t1));
      console.log("delay(s) : " + (t6-t4-500));
      console.log("delay(m) : " + (t7-t6));
      console.log("delay(s>m(p)) : " + (t6-t2-500));
      console.log("delay(s>m) : " + (t10-t9));


    }else if(frame.data[0]==MEASURE_OK){
      for(i=0;i<Number(frame.data[1]);i++){
        j = (i*5)+2
        console.log("type :" + frame.data[j]);

        console.log("ts :"+ Number(frame.data[j+1])+ Number(Number(frame.data[j+2])<<8) + Number(Number(frame.data[j+3])<<16) + Number(Number(frame.data[j+4])<<24));

      }
    }
  }
});



