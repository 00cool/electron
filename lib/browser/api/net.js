'use strict'

const url = require('url')
const {EventEmitter} = require('events')
const util = require('util')
const binding = process.atomBinding('net')
const {net, Net} = binding
const {URLRequest} = net

Object.setPrototypeOf(Net.prototype, EventEmitter.prototype)
Object.setPrototypeOf(URLRequest.prototype, EventEmitter.prototype)

let kSupportedProtocols = new Set()
kSupportedProtocols.add('http:')
kSupportedProtocols.add('https:')

class IncomingMessage extends EventEmitter {
  constructor (urlRequest) {
    super()
    this._url_request = urlRequest
  }

  get statusCode () {
    return this._url_request.statusCode
  }

  get statusMessage () {
    return this._url_request.statusMessage
  }

  get headers () {
    return this._url_request.rawResponseHeaders
  }

  get httpVersion () {
    return `${this.httpVersionMajor}.${this.httpVersionMinor}`
  }

  get httpVersionMajor () {
    return this._url_request.httpVersionMajor
  }

  get httpVersionMinor () {
    return this._url_request.httpVersionMinor
  }

  get rawHeaders () {
    return this._url_request.rawResponseHeaders
  }

}

URLRequest.prototype._emitRequestEvent = function (async, ...rest) {
  if (async) {
    process.nextTick(() => {
      this._request.emit.apply(this._request, rest)
    })
  } else {
    this._request.emit.apply(this._request, rest)
  }
}

URLRequest.prototype._emitResponseEvent = function (async, ...rest) {
  if (async) {
    process.nextTick(() => {
      this._request.emit.apply(this._request, rest)
    })
  } else {
    this._response.emit.apply(this._response, rest)
  }
}

class ClientRequest extends EventEmitter {

  constructor (options, callback) {
    super()

    if (typeof options === 'string') {
      options = url.parse(options)
    } else {
      options = util._extend({}, options)
    }

    const method = (options.method || 'GET').toUpperCase()
    let urlStr = options.url

    if (!urlStr) {
      let urlObj = {}
      const protocol = options.protocol || 'http'
      if (!kSupportedProtocols.has(protocol)) {
        throw new Error('Protocol "' + protocol + '" not supported. ')
      }
      urlObj.protocol = protocol

      if (options.host) {
        urlObj.host = options.host
      } else {
        if (options.hostname) {
          urlObj.hostname = options.hostname
        } else {
          urlObj.hostname = 'localhost'
        }

        if (options.port) {
          urlObj.port = options.port
        }
      }

      const path = options.path || '/'
      if (options.path && / /.test(options.path)) {
        // The actual regex is more like /[^A-Za-z0-9\-._~!$&'()*+,;=/:@]/
        // with an additional rule for ignoring percentage-escaped characters
        // but that's a) hard to capture in a regular expression that performs
        // well, and b) possibly too restrictive for real-world usage. That's
        // why it only scans for spaces because those are guaranteed to create
        // an invalid request.
        throw new TypeError('Request path contains unescaped characters.')
      }
      urlObj.path = path
      urlStr = url.format(urlObj)
    }

    const sessionName = options.session || ''
    let urlRequest = new URLRequest({
      method: method,
      url: urlStr,
      session: sessionName
    })

    // Set back and forward links.
    this._url_request = urlRequest
    urlRequest._request = this

    if (options.headers) {
      const keys = Object.keys(options.headers)
      for (let i = 0, l = keys.length; i < l; i++) {
        const key = keys[i]
        this.setHeader(key, options.headers[key])
      }
    }

    // Set when the request uses chunked encoding. Can be switched
    // to true only once and never set back to false.
    this._chunkedEncoding = false

    // This is a copy of the extra headers structure held by the native
    // net::URLRequest. The main reason is to keep the getHeader API synchronous
    // after the request starts.
    this._extra_headers = {}

    urlRequest.on('response', () => {
      const response = new IncomingMessage(urlRequest)
      urlRequest._response = response
      this.emit('response', response)
    })

    if (callback) {
      this.once('response', callback)
    }
  }

  get chunkedEncoding () {
    return this._chunkedEncoding
  }

  set chunkedEncoding (value) {
    if (!this._url_request.notStarted) {
      throw new Error('Can\'t set the transfer encoding, headers have been sent.')
    }
    this._chunkedEncoding = value
  }

  setHeader (name, value) {
    if (typeof name !== 'string') {
      throw new TypeError('`name` should be a string in setHeader(name, value).')
    }
    if (value === undefined) {
      throw new Error('`value` required in setHeader("' + name + '", value).')
    }
    if (!this._url_request.notStarted) {
      throw new Error('Can\'t set headers after they are sent.')
    }

    const key = name.toLowerCase()
    this._extra_headers[key] = value
    this._url_request.setExtraHeader(name, value)
  }

  getHeader (name) {
    if (arguments.length < 1) {
      throw new Error('`name` is required for getHeader(name).')
    }

    if (!this._extra_headers) {
      return
    }

    const key = name.toLowerCase()
    return this._extra_headers[key]
  }

  removeHeader (name) {
    if (arguments.length < 1) {
      throw new Error('`name` is required for removeHeader(name).')
    }

    if (!this._url_request.notStarted) {
      throw new Error('Can\'t remove headers after they are sent.')
    }

    const key = name.toLowerCase()
    delete this._extra_headers[key]
    this._url_request.removeExtraHeader(name)
  }

  _write (chunk, encoding, callback, isLast) {
    let chunkIsString = typeof chunk === 'string'
    let chunkIsBuffer = chunk instanceof Buffer
    if (!chunkIsString && !chunkIsBuffer) {
      throw new TypeError('First argument must be a string or Buffer.')
    }

    if (chunkIsString) {
      // We convert all strings into binary buffers.
      chunk = Buffer.from(chunk, encoding)
    }

    // Headers are assumed to be sent on first call to _writeBuffer,
    // i.e. after the first call to write or end.
    let result = this._url_request.write(chunk, isLast)

    // Since writing to the network is asynchronous, we conservatively
    // assume that request headers are written after delivering the first
    // buffer to the network IO thread.
    if (!this._url_request.notStarted) {
      this._url_request.setChunkedUpload(this.chunkedEncoding)
    }

    // The write callback is fired asynchronously to mimic Node.js.
    if (callback) {
      process.nextTick(callback)
    }

    return result
  }

  write (data, encoding, callback) {
    if (this._url_request.finished) {
      let error = new Error('Write after end.')
      process.nextTick(writeAfterEndNT, this, error, callback)
      return true
    }

    return this._write(data, encoding, callback, false)
  }

  end (data, encoding, callback) {
    if (this._url_request.finished) {
      return false
    }

    if (typeof data === 'function') {
      callback = data
      encoding = null
      data = null
    } else if (typeof encoding === 'function') {
      callback = encoding
      encoding = null
    }

    data = data || ''

    return this._write(data, encoding, callback, true)
  }

  abort () {
    this._url_request.cancel()
  }

}

function writeAfterEndNT (self, error, callback) {
  self.emit('error', error)
  if (callback) callback(error)
}

Net.prototype.request = function (options, callback) {
  return new ClientRequest(options, callback)
}

net.ClientRequest = ClientRequest

module.exports = net
