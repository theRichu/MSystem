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

var DEVICE_NUM=2;

var t1=new Array();
var t2=new Array();
var t4=new Array();
var t6=new Array();
var t7=new Array();
var t9=new Array();
var t10=new Array();


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


  var frame_TEST_DRILL_1 = {
    type: 0x01,
    id: 0x01,
    destination16:"0001",
    options: 0x01,
    data: [ DRILL_READY, 0x40, 0x01, 0x02, 0x02, 0x00, 0x10 ]
  };

    var frame_TEST_DRILL_3 = {
    type: 0x01,
    id: 0x01,
    destination16:"0003",
    options: 0x01,
    data: [ DRILL_READY, 0x40, 0x01, 0x02, 0x02, 0x00, 0x30 ]
  };


  serialport.write(xbeeAPI.buildFrame(frame_RESET), function(err, res) {
  if (err) throw(err);
  else console.log ("reset");
  });

//Sync
  setTimeout(function () {
    t1[0]=new Date().getTime();
    t1[3]=new Date().getTime();
    serialport.write(xbeeAPI.buildFrame(frame_SYNC), function(err, res) {
    if (err) throw(err);
    else     console.log("sync: "+util.inspect(res));
  });
}, 1000);
     
  setTimeout(function(){
	  serialport.write(xbeeAPI.buildFrame(frame_TEST_DRILL_1), function(err, res) {
    if (err) throw(err);
    else     console.log("test drill:"+util.inspect(res));
   });

    serialport.write(xbeeAPI.buildFrame(frame_TEST_DRILL_3), function(err, res) {
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
      t4[frame.remote16] = time_temp;
      console.log("t4 : " + t4[frame.remote16]);
    }else if(frame.data[0]==DELAY_REQ){
        t7[frame.remote16] = new Date().getTime();
        console.log("t7 : " + t7[frame.remote16]);
        frame_DELAY_RESP.destination16 = frame.remote16;
        serialport.write(xbeeAPI.buildFrame(frame_DELAY_RESP), function(err, res) {
          if (err) throw(err);
          else  console.log("DELAY_RESP: "+util.inspect(res));
        });
      t6[frame.remote16] = time_temp;
      console.log("t6 : " + t6[frame.remote16]);
      t2[frame.remote16] = Number(frame.data[1])+ Number(Number(frame.data[2])<<8) + Number(Number(frame.data[3])<<16) + Number(Number(frame.data[4])<<24);
      
      console.log("t2 : " + t2[frame.remote16]);
    }else if(frame.data[0]==DELAY_FEEDBACK){
      t10[frame.remote16] = time_temp;
      console.log("t10 : " + t10[frame.remote16]);
      t9[frame.remote16] = Number(frame.data[1])+ Number(Number(frame.data[2])<<8) + Number(Number(frame.data[3])<<16) + Number(Number(frame.data[4])<<24);
      console.log("t9 : " + t9[frame.remote16]);
      //Time Sync Complete

      console.log("delay(m>s) : " + (t2[frame.remote16]-t1[frame.remote16]));
      console.log("delay(s) : " + (t6[frame.remote16]-t4[frame.remote16]-500));
      console.log("delay(m) : " + (t7[frame.remote16]-t6[frame.remote16]));
      console.log("delay(s>m(p)) : " + (t6[frame.remote16]-t2[frame.remote16]-500));
      console.log("delay(s>m) : " + (t10[frame.remote16]-t9[frame.remote16]));


    }else if(frame.data[0]==MEASURE_OK){
      for(i=0;i<Number(frame.data[1]);i++){
        j = (i*5)+2
        console.log("type :" + frame.data[j]);

        console.log("ts :"+ Number(frame.data[j+1])+ Number(Number(frame.data[j+2])<<8) + Number(Number(frame.data[j+3])<<16) + Number(Number(frame.data[j+4])<<24));

      }
    }
  }
});



