var SerialPort = require("serialport").SerialPort
var serialPort = new SerialPort("/dev/ttyUSB0", {
  baudrate: 57600
}, false); // this is the openImmediately flag [default is true]

serialPort.on('error', function(err) {
  console.log(err); // THIS SHOULD WORK!
});

serialPort.open(function (err) {
  if (err) {
     console.log(err);
     return;
  }
  console.log('open');
  serialPort.on('data', function(data) {
    console.log('data received: ' + data);
  });  
  serialPort.write("ls\n", function(err, results) {
    console.log('err ' + err);
    console.log('results ' + results);
  });  
});
