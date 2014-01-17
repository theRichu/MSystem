// 모듈을 추출합니다.
var fs = require('fs');
var http = require('http');
var express = require('express');
var util = require('util');
var SerialPort = require('serialport').SerialPort;
var xbee_api = require('./lib/xbee-api.js');

// 데이터베이스와 연결합니다.
var client = require('mysql').createConnection({
  user: 'coach',
  password: 'coach',
  database: 'C'
});


//ORDER_CODE
var SYNC = 0x10;
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


// 웹 서버를 생성합니다.
var app = express();
var server = http.createServer(app);

// Xbee Serial Port
var C = xbee_api.constants;

var xbeeAPI = new xbee_api.XBeeAPI({
  api_mode: 1
});

var serialport = new SerialPort("/dev/ttyUSB0", {
  baudrate: 9600,
  parser: xbeeAPI.rawParser()
});

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
  data: [SYNC]
};

var frame_DELAY_RESP = {
  type: 0x01,
  id: 0x01,
  destination16: "FFFF",
  options: 0x01,
  data: [DELAY_RESP]
};

var frame_TEST_DRILL = {
  type: 0x01,
  id: 0x01,
  destination16: "FFFF",
  options: 0x01,
  data: [DRILL_READY, 0x40, 0x02, 0x02, 0x02]
};


xbeeAPI.on("frame_object", function (frame) {
  var TX16 = 0x81;
  //var 
  //  console.log("OBJ> "+util.inspect(frame));
  if (frame.type == TX16) {
    var resp_code = Number(frame.data[0]);

    console.log("Remote16 : " + frame.remote16 + "  Response : 0x" + resp_code.toString(16));

    switch (resp_code) {
    case RESP:
      //Device is alive
      //Update DB - device;
      client.query('UPDATE device set iddevice=?, status=?, time=?', [frame.remote16, 1]);

      break;


    case DELAY_REQ:
      //Calc delay(m->s) and delay(s)
      //SAVE TIMESTAMP
      //SEND DELAY_RESP
      serialport.on("open", function () {
        frame_DELAY_RESP.destination16 = frame.remote16;
        serialport.write(xbeeAPI.buildFrame(frame_DELAY_RESP), function (err, res) {
          if (err) throw (err);
          else console.log("ERROR_DELAY_RESP");
        });
      });

      break;
    case DELAY_FEEDBACK:
      //calc delay(s->m)
      break;
    case MEASURE_START:
      //CHANGE STATUS 
      break;

    case MEASURE_END:
      //Save DB - record
      break;
    case MEASURE_OK:
      //Save DB - record
      for(i=0;i<Number(frame.data[1]);i++){
        k=2+i*5;
        client.query('INSERT INTO record(sensor_type, data, timestamp, time) VALUES(?, ?, ?, ?, now())', [frame.data[k]|0xF0,frame.data[k]|0xF,        (frame.data[k+1]) + (frame.data[k+2]<<8) + (frame.data[k+3]<<16) + (frame.data[k+4]<<24)]);
      }

      break;

    case MEASURE_READYD:
      //Update Status - device
      break;
    case MEASURE_RESETTED:
      //Update Status - device
      break;
    case STATUS:
      //Update Status - device
      break;


    }
  }
});


// GET - /enroll
app.get('/enroll', function (request, response) {
  //enroll.html 파일을 제공합니다.
  fs.readFile('enroll.html', function (error, data) {
    response.send(data.toString());
  });
});

app.get('/', function (request, response) {
  fs.readFile('index.html', function (error, data) {
    response.send(data.toString());
  });
});

app.get('/check_status', function (request, response) {
  serialport.write(xbeeAPI.buildFrame(frame_RESET), function (err, res) {
    if (err) throw (err);
    else console.log("reset");
  });

  setTimeout(function () {
    serialport.write(xbeeAPI.buildFrame(frame_SYNC), function (err, res) {
      if (err) throw (err);
      else console.log("sync: " + util.inspect(res));
    });
  }, 1000);
  setTimeout(function () {
    serialport.write(xbeeAPI.buildFrame(frame_DELAY_RESP), function (err, res) {
      if (err) throw (err);
      else console.log("DELAY_RESP: " + util.inspect(res));
    });

  }, 4000);

  setTimeout(function () {
    serialport.write(xbeeAPI.buildFrame(frame_TEST_DRILL), function (err, res) {
      if (err) throw (err);
      else console.log("test drill:" + util.inspect(res));
    });

  }, 5000);

});


app.get('/set_drill', function (request, response) {

  serialport.on("open", function () {
    console.log("Set drill");
    serialport.write(xbeeAPI.buildFrame(frame_TEST_DRILL), function (err, res) {
      if (err) throw (err);
      else console.log("reset");
    });
  });

  //Sent SYNC.. 
});


app.get('/start_trainee', function (request, response) {

  serialport.on("open", function () {
    console.log("Trainee Number");
    serialport.write(xbeeAPI.buildFrame(frame_SYNC), function (err, res) {
      if (err) throw (err);
      else console.log("reset");
    });
  });

  //Sent SYNC.. 
});


// GET - /tracker
app.get('/tracker', function (request, response) {
  // Tracker.html 파일을 제공합니다.
  fs.readFile('Tracker.html', function (error, data) {
    response.send(data.toString());
  });
});

// GET - /observer
app.get('/observer', function (request, response) {
  // Observer.html 파일을 제공합니다.
  fs.readFile('Observer.html', function (error, data) {
    response.send(data.toString());
  });
});

// GET - /ShowData
app.get('/showtrainee', function (request, response) {
  // 데이터베이스의 데이터를 제공합니다.
  client.query('SELECT * FROM trainee WHERE name=?', [request.param('name')], function (error, data) {
    response.send(data);
  });
});

app.get('/showrecord', function (request, response) {
  client.query('SELECT * FROM record WHERE fk_trainee=?', [request.param('traineeid')], function (error, data) {
    response.send(data);
  });
});

// 웹 서버를 실행합니다.
server.listen(52273, function () {
  console.log('Server Running at http://127.0.0.1:52273');
});

// 소켓 서버를 생성 및 실행합니다.
var io = require('socket.io').listen(server);
io.sockets.on('connection', function (socket) {

  // location 이벤트
  socket.on('enroll_trainee', function (data) {

    // 데이터를 삽입합니다.
    client.query('INSERT INTO trainee(name, age, gender, type, created_time) VALUES (?, ?, ?, ?,NOW())', [data.name, data.age, data.gender, data.type]);

    // receive 이벤트를 발생시킵니다.            
    io.sockets.emit('trainee_receive', {
      name: data.name,
      age: data.age,
      gender: data.gender,
      type: data.type
    });
  });


  // device_sync
  socket.on('device_sync', function (data) {

    serialport.write(xbeeAPI.buildFrame(frame_SYNC), function (err, res) {
      if (err) throw (err);
      else console.log("sync: " + util.inspect(res));
    });
    setTimeout(function () {
      serialport.write(xbeeAPI.buildFrame(frame_DELAY_RESP), function (err, res) {
        if (err) throw (err);
        else console.log("DELAY_RESP: " + util.inspect(res));
      });

    }, 4000);


    client.query('SELECT * FROM device', function (error, data) {
      //console.log(data);
      // receive 이벤트를 발생시킵니다.            
      io.sockets.emit('status_receive', data);
    });
  });


  socket.on('drill_set', function (data) {
    serialport.write(xbeeAPI.buildFrame(frame_TEST_DRILL), function (err, res) {
      if (err) throw (err);
      else console.log("test drill:" + util.inspect(res));
    });
  });
  socket.on('device_reset', function (data) {
    serialport.write(xbeeAPI.buildFrame(frame_RESET), function (err, res) {
      if (err) throw (err);
      else console.log("reset");
    });
  });


});
// socket.on('check_status');
/*
    // location 이벤트
    socket.on('location', function (data) {
        // 데이터를 삽입합니다.
        client.query('INSERT INTO locations(name, latitude, longitude, date) VALUES (?, ?, ?, NOW())', [data.name, data.latitude, data.longitude]);


        // receive 이벤트를 발생시킵니다.            
        io.sockets.in(data.name).emit('receive', {
            latitude: data.latitude,
            longitude: data.longitude,
            date: Date.now()
        });
    });
*/