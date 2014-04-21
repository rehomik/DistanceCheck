
/**
 * Module dependencies.
 */

var express = require('express');
var check_v1 = require('./routes/v1/check');
var display_v1 = require('./routes/v1/display');
var http = require('http');
var path = require('path');

var app = express();

// all environments
app.set('port', process.env.PORT || 8093);
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');
app.use(express.favicon());
app.use(express.logger('dev'));
app.use(express.json());
app.use(express.bodyParser());
app.use(express.urlencoded());
app.use(express.methodOverride());
app.use(app.router);
app.use(express.static(path.join(__dirname, 'public')));

// development only
if ('development' == app.get('env')) {
  app.use(express.errorHandler());
}

// RESTful api
app.post('/v1/check/checkin', check_v1.checkIn);

// return page
app.get("/", display_v1.index);
app.get('/v1/display', display_v1.displayState);
app.get("/v1/displayStats", display_v1.displayStats);

http.createServer(app).listen(app.get('port'), function(){

  console.log('Professor checker server listening on port ' + app.get('port'));
});