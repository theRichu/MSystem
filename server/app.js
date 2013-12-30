/**
 * Module dependencies.
 */

var express = require('express');
var routes = require('./routes');
var user = require('./routes/user');
var http = require('http');
var path = require('path');

var app = express();

var mongoose = require('mongoose').connect('mongodb://localhost/M');
var Measure = require('./data/models/measure');

// all environments
app.set('port', process.env.PORT || 3000);
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');
app.use(express.favicon());
app.use(express.logger('dev'));
app.use(express.json());
app.use(express.urlencoded());
app.use(express.methodOverride());
app.use(app.router);
app.use(express.static(path.join(__dirname, 'public')));

// development only
if ('development' === app.get('env')) {
  app.use(express.errorHandler());
}

//app.get('/', routes.index);
//app.get('/users', user.list);
app.post('/test', function (req, res) {
  res.end('It worked!');

  var measure = new Measure({type: req.body['type'], data: req.body['data']});

//save model to MongoDB
measure.save(function (err) {
  if (err) {
    return err;
  }
  else {
    console.log("Post saved");
  }
});
  console.log(JSON.stringify(req.body));

});

http.createServer(app).listen(app.get('port'), function () {
  console.log('Express server listening on port ' + app.get('port'));
});
