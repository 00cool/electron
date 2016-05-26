const {app, protocol, webContents, BrowserWindow} = require('electron')
const fs = require('fs')
const path = require('path')
const url = require('url')

// TODO(zcbenz): Remove this when we have Object.values().
const objectValues = function (object) {
  return Object.keys(object).map(function (key) { return object[key] })
}

// Mapping between hostname and file path.
let hostPathMap = {}
let hostPathMapNextKey = 0

const generateHostForPath = function (path) {
  let key = `extension-${++hostPathMapNextKey}`
  hostPathMap[key] = path
  return key
}

const getPathForHost = function (host) {
  return hostPathMap[host]
}

// Cache manifests.
let manifestMap = {}

const getManifestFromPath = function (srcDirectory) {
  let manifest = JSON.parse(fs.readFileSync(path.join(srcDirectory, 'manifest.json')))
  if (!manifestMap[manifest.name]) {
    const hostname = generateHostForPath(srcDirectory)
    manifestMap[manifest.name] = manifest
    Object.assign(manifest, {
      srcDirectory: srcDirectory,
      hostname: hostname,
      // We can not use 'file://' directly because all resources in the extension
      // will be treated as relative to the root in Chrome.
      startPage: url.format({
        protocol: 'chrome-extension',
        slashes: true,
        hostname: hostname,
        pathname: manifest.devtools_page
      })
    })
    return manifest
  }
}

// Manage the background pages.
let backgroundPages = {}

const startBackgroundPages = function (manifest) {
  if (backgroundPages[manifest.hostname] || !manifest.background) return

  const scripts = manifest.background.scripts.map((name) => {
    return `<script src="${name}"></script>`
  }).join('')
  const html = new Buffer(`<html><body>${scripts}</body></html>`)

  const contents = webContents.create({})
  backgroundPages[manifest.hostname] = { html: html, contents: contents }
  contents.loadURL(url.format({
    protocol: 'chrome-extension',
    slashes: true,
    hostname: manifest.hostname,
    pathname: '_generated_background_page.html'
  }))
}

// Transfer the |manifest| to a format that can be recognized by the
// |DevToolsAPI.addExtensions|.
const manifestToExtensionInfo = function (manifest) {
  return {
    startPage: manifest.startPage,
    srcDirectory: manifest.srcDirectory,
    name: manifest.name,
    exposeExperimentalAPIs: true
  }
}

// Load the extensions for the window.
const loadDevToolsExtensions = function (win, manifests) {
  if (!win.devToolsWebContents) return

  for (let manifest of manifests) {
    startBackgroundPages(manifest)
  }
  const extensionInfoArray = manifests.map(manifestToExtensionInfo)
  win.devToolsWebContents.executeJavaScript(`DevToolsAPI.addExtensions(${JSON.stringify(extensionInfoArray)})`)
}

// The persistent path of "DevTools Extensions" preference file.
let loadedExtensionsPath = null

app.on('will-quit', function () {
  try {
    let loadedExtensions = objectValues(manifestMap).map(function (manifest) {
      return manifest.srcDirectory
    })
    if (loadedExtensions.length > 0) {
      try {
        fs.mkdirSync(path.dirname(loadedExtensionsPath))
      } catch (error) {
        // Ignore error
      }
      fs.writeFileSync(loadedExtensionsPath, JSON.stringify(loadedExtensions))
    } else {
      fs.unlinkSync(loadedExtensionsPath)
    }
  } catch (error) {
    // Ignore error
  }
})

// We can not use protocol or BrowserWindow until app is ready.
app.once('ready', function () {
  // Load persisted extensions.
  loadedExtensionsPath = path.join(app.getPath('userData'), 'DevTools Extensions')
  try {
    let loadedExtensions = JSON.parse(fs.readFileSync(loadedExtensionsPath))
    if (Array.isArray(loadedExtensions)) {
      // Preheat the manifest cache.
      for (let srcDirectory of loadedExtensions) {
        getManifestFromPath(srcDirectory)
      }
    }
  } catch (error) {
    // Ignore error
  }

  // The chrome-extension: can map a extension URL request to real file path.
  const chromeExtensionHandler = function (request, callback) {
    let parsed = url.parse(request.url)
    if (!parsed.hostname || !parsed.path) return callback()
    if (!/extension-\d+/.test(parsed.hostname)) return callback()

    let directory = getPathForHost(parsed.hostname)
    if (!directory) return callback()

    if (parsed.path === '/_generated_background_page.html' &&
        backgroundPages[parsed.hostname]) {
      return callback({
        mimeType: 'text/html',
        data: backgroundPages[parsed.hostname].html
      })
    }

    fs.readFile(path.join(directory, parsed.path), function (err, content) {
      if (err)
        callback(-6)  // FILE_NOT_FOUND
      else
        return callback({mimeType: 'text/html', data: content})
    })
  }
  protocol.registerBufferProtocol('chrome-extension', chromeExtensionHandler, function (error) {
    if (error) {
      console.error(`Unable to register chrome-extension protocol: ${error}`)
    }
  })

  BrowserWindow.addDevToolsExtension = function (srcDirectory) {
    const manifest = getManifestFromPath(srcDirectory)
    if (manifest) {
      for (let win of BrowserWindow.getAllWindows()) {
        loadDevToolsExtensions(win, [manifest])
      }
      return manifest.name
    }
  }
  BrowserWindow.removeDevToolsExtension = function (name) {
    delete manifestMap[name]
  }

  // Load persisted extensions when devtools is opened.
  let init = BrowserWindow.prototype._init
  BrowserWindow.prototype._init = function () {
    init.call(this)
    this.webContents.on('devtools-opened', () => {
      loadDevToolsExtensions(this, objectValues(manifestMap))
    })
  }
})
