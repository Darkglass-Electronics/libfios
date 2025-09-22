import path from 'path'
import {SerialPort} from 'serialport'
const fios = require(path.join(__dirname, '../build/Release/fios.node'))
console.log('fios', fios)

const checkForPath = async () => {
  const list = await SerialPort.list()
  let foundSerialDevice = list.find(
    (d) => d.vendorId?.toLowerCase() === '2fa6' && d.productId === '2500'
  )

  if (!foundSerialDevice) {
    foundSerialDevice = list.filter(
      (d) => d.vendorId?.toLowerCase() === '2fa6' && d.productId === '2500'
    )[0]
  }
  let serialPath = foundSerialDevice?.path

  if (!serialPath && !list[0]?.path) {
    console.log(new Error('Unable to find Serial Port!'))
    return
  }
  if (serialPath.includes('COM')) {
    const compPort = serialPath.split('COM').pop()
    if (!isNaN(Number(compPort)) && Number(compPort) > 9) {
      serialPath = String.raw`\\.\{port}`.replace('{port}', serialPath)
    } else {
      console.log('keeping old path for COM port under 10', compPort)
    }
  }
  console.log('serialPath:', serialPath)
  return serialPath
}

const main = async () => {
  const devpath = await checkForPath()

  // Open the path
  console.log('Opening with devpath...', devpath)
  const s = fios.fios_serial_open(devpath)
  console.log('fios_serial_open', s)

  console.log('test sending a file')
  const f = fios.fios_file_send(s, path.join(__dirname, 'test.nam'))
  if (!f) {
    if (s !== null) {
      fios.fios_serial_close(s)
    }
    console.error(new Error('Unable to detect file'))
  }

  // check progress of sending file
  console.log('Checking progress...')
  let progress = fios.new_float_ptr()
  let failedCount = 0

  const loop = async () => {
    console.log('f idle', f)
    if (!f || f === null) {
      return
    }
    let status = fios.fios_file_idle(f, progress)
    console.log(
      'loop status',
      status,
      'last progress',
      fios.get_float_ptr_value(progress)
    )
    if (status === 1) {
      process.stdout.write(
        `\rProgress: ${(fios.get_float_ptr_value(progress) * 100).toFixed(1)} %`
      )
      console.log(
        'sending file...: ' +
          (fios.get_float_ptr_value(progress) * 100).toFixed(1)
      )
      setTimeout(loop, 100)
      return
    } else {
      console.log('status', status)
      console.log('shutting down...')
      console.log('sending file shutting down: ' + status)
      fios.delete_float_ptr(progress)
      fios.fios_file_close(f)
      if (s !== null) {
        fios.fios_serial_close(s)
      }
      if (status === 2) {
        console.log('sending file success')
      } else {
        console.log('sending file error')
      }
    }
  }
  loop()
  // Close the port after
  // const temp = fios.fios_serial_close(s)
  // console.log('Closed', temp)
}

main()
