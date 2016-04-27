/*setInterval(function() {function error_proc () {
  console.log('[Error] error when trying to unrar file')
}

function terminate_proc () {
  console.log('[Terminate] terminate unraring file')
}

function finished_proc (filename) {
  console.log('[Finished] finished unrar file')
  console.log(filename)
}

function process_proc (val, filename) {
  console.log('[process] process %d %', val)
  console.log('[process]', filename)
}

function fail_proc () {
  console.log('[Fail] fail when trying to unrar file')
}

var Unzip = require('./unzip.node').Unzipper
var obj = new Unzip(__dirname+'/ToSend.zip', __dirname+'/files')

obj.run(error_proc, terminate_proc, process_proc)
},2000);
*/
function error_proc () {
  console.log('[Error] error when trying to unrar file')
}

function terminate_proc () {
  console.log('[End] ending unzipping file')
}

function finished_proc (filename) {
  console.log('[Finished] finished unrar file')
  console.log(filename)
}

function process_proc (val, filename) {
  console.log('[process] process %d %', val)
  console.log('[process]', filename)
}


var Unzip = require('./unzip.node').Unzipper
var obj = new Unzip(__dirname+'/ToSend.zip', __dirname+'/files')

obj.run(error_proc, terminate_proc, process_proc)