/*
Jabra Browser Integration
https://github.com/gnaudio/jabra-browser-integration

MIT License

Copyright (c) 2017 GN Audio A/S (Jabra)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Native messages port
var port = null;

// Open the NM port
function initNmPortIfNotOpen() {
  if (port == null) {
    var hostName = "com.jabra.nm";
    port = window.chrome.runtime.connectNative(hostName);
    port.onMessage.addListener(onNativeMessageReceived);
    port.onDisconnect.addListener(onDisconnected);
  }  
}

// Native app was disconnected
function onDisconnected() {
  sendErrorToContentScript("You need to install the <a href='https://gnaudio.github.io/jabra-browser-integration/download'>Jabra Browser Integration Host</a> and reload this page");
  sendMessageToPopupScript("Disconnected");
  port = null;
}

// Send message to native app
function sendMessageToNativeApp(message) {
  port.postMessage({ message: message });
}

// Message from native app
function onNativeMessageReceived(message) {
  if (message.message.startsWith("Event: Version ")) {
    // Supported versions: 0.5
    if (!(message.message === "Event: Version 0.5")) {
      sendErrorToContentScript("You need to upgrade the <a href='https://gnaudio.github.io/jabra-browser-integration/download'>Jabra Browser Integration Host</a> and reload this page");
      return;
    }
  }

  // Popup
  if (message.message.startsWith("Event: device attached") || message.message.startsWith("Event: device detached")) {
    sendMessageToNativeApp("getdevices");
  }
  if (message.message.startsWith("Event: devices")) {
    sendMessageToPopupScript(message.message);
  }

  sendMessageToContentScript(message.message);
}

// Send a message to the popup-script
function sendMessageToPopupScript(message) {
  window.chrome.extension.sendRequest({ txt: message }, function (response) {
    // response.result
  });
  
}

// Send a message to the content-script
function sendMessageToContentScript(message) {
  window.chrome.tabs.query({
  }, function (tabs) {
    tabs.forEach(function (tab) {
      window.chrome.tabs.sendMessage(tab.id, { message: message });
    });
  });
}

// Send a error-message to content-script
function sendErrorToContentScript(err) {
  window.chrome.tabs.query({
  }, function (tabs) {
    tabs.forEach(function (tab) {
      window.chrome.tabs.sendMessage(tab.id, { error: err });
    });
  });
}

(function () {
  // Subscribe to messages from the content-script
  window.chrome.runtime.onMessage.addListener(function (request) {
    initNmPortIfNotOpen();
    sendMessageToNativeApp(request.message);
  });
  // Subscribe to messages from extensions pages
  window.chrome.extension.onRequest.addListener(function (request, sender, sendResponse) {
    initNmPortIfNotOpen();
    sendMessageToNativeApp("getdevices");
  });

})();