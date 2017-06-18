var express = require('express')
var keyboard = require('robotjs')
var bodyParser = require('body-parser')
var serialport = require('serialport')
var portName = process.argv[2]
var port = new serialport(portName, {
  baudRate: 9600
})

var x_LPF, y_LPF, z_LPF;
var alpha = 0.3;

var obj = {
  x: 0,
  y: 0,
  z: 0
}

function calculateAngle()
{
  x_LPF = obj.x * alpha + (x_LPF*(1 - alpha))
  y_LPF = obj.y * alpha + (y_LPF*(1 - alpha))
  z_LPF = obj.z * alpha + (z_LPF*(1 - alpha))
  pitch = Math.atan2((- x_LPF) , Math.sqrt(y_LPF * y_LPF + z_LPF * z_LPF)) * 57.3

  return pitch
}

port.on('open', function(){
  console.log('Puerto ' + portName + ' abierto...')
})

var app = express()

app.use(bodyParser())

app.get('/', function(req, res) {
  res.send('Home Page')
  console.log('Nice shake man')
})

app.get('/lights', function(req, res) {
  res.send('Lights toggled!')
  console.log('yeah lights!')
  port.write('a')
})

app.get('/user/:user', function(req, res) {
  res.send('Hi ' + req.params.user + '!!!')
  console.log('Hi ' + req.params.user + '!!!')
  var temp = req.params.user.toString().split(':');
  if(temp[0] == 0){
    obj.x = temp[1];
  }
  else if(temp[0] == 1){
    obj.y = temp[1];
  }
  else {
    obj.z = temp[1];
    console.log('Angle: ' + calculateAngle())
  }
})

app.get('/desktop/lock', function(req, res) {
  res.send('Locking computer!')

})

app.post('/login', function(req, res) {
  console.log(req.body.user)
  res.send('Thank you for login in ' + req.body.user)
})

app.get('/desktop/slides/:action', function(req, res) {
  if(req.params.action.toString() == 'right')
  {
    res.send('Keep Presenting')
    console.log('right')
    keyboard.keyTap('right')
  }
  else
  {
    res.send('Going Backwards!')
    console.log('left')
    keyboard.keyTap('left')
  }
})

var server = app.listen(portName)

console.log('Servidor corriendo en el puerto ' + portName)
