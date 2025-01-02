const {
  fios_serial_open,
  fios_serial_close,
  fios_file_receive,
  fios_file_send,
  fios_file_idle,
  fios_file_close,
  new_float_ptr,
  get_float_ptr_value,
  delete_float_ptr,
} = require('./build/Release/fios.node')

function usage() {
  console.error(`Usage: ${ process.argv[1] } [r|s] [device-path] [file-path]`)
  process.exit(1)
}

if (process.argv.length !== 5) {
  usage()
}

let sending;
if (process.argv[2] == 'r')
  sending = false
else if (process.argv[2] == 's')
  sending = true
else
  usage()

const devpath = process.argv[3]
const filepath = process.argv[4]

const s = fios_serial_open(devpath)

if (! s)
  process.exit(2)

const f = sending ? fios_file_send(s, filepath)
                  : fios_file_receive(s, filepath)

if (! f) {
  fios_serial_close(s);
  process.exit(3)
}

let progress = new_float_ptr()

console.log('');

function loop() {
  if (fios_file_idle(f, progress)) {
    process.stdout.write(`\rProgress: ${ (get_float_ptr_value(progress) * 100).toFixed(1) } %`)
    setTimeout(loop, 100)
    return
  }

  console.log('')
  console.log('shutting down...')

  delete_float_ptr(progress)
  fios_file_close(f)
  fios_serial_close(s)
  process.exit()
}

loop()