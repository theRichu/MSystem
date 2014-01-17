var SerialPort = require("serialport").SerialPort
var serialPort = new SerialPort("/dev/ttyUSB0", {
  baudrate: 57600
});
var serialPort = require("serialport");
serialPort.list(function (err, ports) {
  ports.forEach(function(port) {
    console.log(port.comName);
    console.log(port.pnpId);
    console.log(port.manufacturer);
  });
});
