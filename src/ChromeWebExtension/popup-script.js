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

// DOM loaded
document.addEventListener('DOMContentLoaded', function () {

//  function updateValue(txt) {
//    document.getElementById('value').innerHTML = txt;
//  }
//
//  updateValue("Loading...");
//  console.log("Loading...");


//  var bkg = window.chrome.extension.getBackgroundPage(); 
//  var res = bkg.ping("offhook");
//  updateValue("res = " + res);
//  console.log("res = " + res);

  document.getElementById("deviceDiv").style.display = 'none'; 
  document.getElementById("alertDiv").style.display = 'none'; 


  window.chrome.extension.onRequest.addListener(function (request, sender, sendResponse) {

    if (request.txt === "Disconnected") {
      document.getElementById("alertDiv").style.display = 'block'; 
    } else {

      var subTxt = request.txt.substring(15);
      var result = subTxt.split(",");

      var listObject = document.getElementById("devices");
      listObject.innerHTML = "";

      if (result.length < 2) {
        listObject.innerHTML = "No devices";
      } else {

        for (var i = 0; i < result.length / 6; i++) {

          var img2 = new Image();
          img2.src = 'data:image/png;base64,' + result[6 * i + 3];
          var li4 = document.createElement('div');
          li4.appendChild(img2);
          listObject.appendChild(li4);

          var element = document.createElement("h3");
          element.innerHTML = result[6 * i + 1];
          var li1 = document.createElement('div');
          li1.appendChild(element);
          listObject.appendChild(li1);

          var element2 = document.createElement("p");
          element2.innerHTML = "Serial: " + result[6 * i + 2];
          var li2 = document.createElement('div');
          li2.appendChild(element2);
          listObject.appendChild(li2);


          var element5 = document.createElement("p");
          element5.innerHTML = "Firmware: " + result[6 * i + 4];
          var li5 = document.createElement('div');
          li5.appendChild(element5);
          listObject.appendChild(li5);

          // Battery?

          if (result[6 * i + 5] !== "Not_Supported") {
            var element3 = document.createElement("p");
            element3.innerHTML = "Battery: " + result[6 * i + 5];
            var li3 = document.createElement('div');
            li3.appendChild(element3);
            listObject.appendChild(li3);
            
          }


        }

      }

      document.getElementById("deviceDiv").style.display = 'block'; 

      
    }


    //updateValue("popup - got something: " + txt);
    //console.log("popup - got something: " + txt);

  });


  window.chrome.extension.sendRequest({ method: 'test'}, function (response) {
    // response.result
  });


}, false);