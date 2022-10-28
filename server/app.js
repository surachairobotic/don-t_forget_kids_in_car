'use strict';

const PORT = 8088
  , IMG_DIR = './img'
  , TMP_DIR = './tmp';

const express = require('express');
const bodyParser = require('body-parser');
var http = require('http');
var multer  = require('multer');
var PATH = require('path');
var multer_upload = multer({ dest: TMP_DIR })
var fs = require('fs').promises;
var app = express();

var server = http.createServer(function (req, res) {
  app(req,res);
});

async function init(){
  try{
    await fs.mkdir( TMP_DIR, {recursive: true} );
  }
  catch(e){}
  try{
    await fs.mkdir( IMG_DIR, {recursive: true} );
  }
  catch(e){}

  init_web_common();
  server.on('error', function (e) {
    console.log('error', null, e);
  });
  server.listen( PORT, '0.0.0.0');
  console.log('start on port : '+PORT);
}

function init_web_common(){
  app.set('port', PORT);
  app.set('etag', false);
  app.disable('x-powered-by');
  app.disable('Server');

  app.use(bodyParser.json());

  app.post('/api/add_image', multer_upload.single('image'), async function (req, res, next) {
    console.log('add_image : '+req.file.originalname);

    //var apikey = arr[1];
    var t = new Date()
      , year = t.getFullYear()-2000
      , month = t.getMonth() + 1
      , date = t.getDate()
      , hour = t.getHours()
      , min = t.getMinutes()
      , sec = t.getSeconds()
      , filename = String(year)
      + (month<10 ? '0' : '') + month
      + (date < 10 ? ('0' + date) : date) + '-'
      + (hour < 10 ? ('0' + hour) : hour)
      + (min < 10 ? ('0' + min) : min)
      + (sec < 10 ? ('0' + sec) : sec)
      + '-' + req.file.originalname;
    //  + '.jpg';
    await fs.copyFile( req.file.path, PATH.join(IMG_DIR, filename) );
    await fs.unlink( req.file.path );
    res.send({
      result: 'success'
    });
  });
}

function api_error(req, res, err){
  var user_id = req.session && req.session.user_id ? req.session.user_id : null;
  var msg = (err && typeof(err)=='string') ? err : 'Unknown error';
  res.send({
    result: 'fail',
    msg: msg
  });
  if( err )
    console.log('api error : ',err);
};

if (require.main === module) {
  init();
}

function create_random_string(len){
  const possible = "abcdefghijklmnpqrstuvwxyz123456789";
  var text = "";
  for (var i = 0; i < len; i++)
    text += possible.charAt(Math.floor(Math.random() * possible.length));
  return text;
}
