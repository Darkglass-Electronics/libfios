const path = require('path')

const fios = require(path.resolve(
  __dirname,
  path.join(__dirname, 'build/Release/fios.node')
))

module.exports = {fios}
