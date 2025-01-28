declare module '*libfios' {
  export class SerialPort {
    devpath: string
  }

  export class FIOSFile {
    serial: SerialPort
    file: any
  }

  export interface FIOS {
    CMD_SIZE: number
    MAX_FILE_SIZE: number
    MAX_PAYLOAD_SIZE: number
    /**
     * @description File Transfer Status
     */
    fios_file_status_error: 0
    fios_file_status_in_progress: 1
    fios_file_status_completed: 2

    /**
     * @description Open the stream (Receiver)
     */
    fios_serial_open: (path: string) => SerialPort

    fios_serial_close: Function
    fios_serial_read_cmd: Function
    fios_serial_read_payload: Function
    fios_serial_write_cmd: Function
    fios_serial_write_payload: Function
    fios_file_receive: Function

    /**
     * @description Send a File (Sender)
     */
    fios_file_send: (s: SerialPort, filePath: string) => FIOSFile

    fios_file_idle: Function
    fios_file_close: Function
    new_float_ptr: Function
    get_float_ptr_value: Function
    delete_float_ptr: Function

    fios_file_get_progress: Function
    fios_file_get_last_error: Function
  }

  /**
   * The default export of the module.
   */
  const fios: FIOS
  export default fios
}
