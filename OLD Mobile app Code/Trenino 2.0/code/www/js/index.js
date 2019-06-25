// (c) 2013-2015 Don Coleman
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/* global mainPage, deviceList, refreshButton, statusDiv */
/* global detailPage, resultDiv, messageInput, sendButton, disconnectButton */
/* global cordova, bluetoothSerial  */
/* jshint browser: true , devel: true*/
'use strict';

var app = {
    initialize: function() {
        this.bindEvents();
        this.showMainPage();
    },
    bindEvents: function() {

        var TOUCH_START = 'touchstart';
        if (window.navigator.msPointerEnabled) { // windows phone
            TOUCH_START = 'MSPointerDown';
        }
        document.addEventListener('deviceready', this.onDeviceReady, false);
        refreshButton.addEventListener(TOUCH_START, this.refreshDeviceList, false);
        //sendButton.addEventListener(TOUCH_START, this.sendData, false);	
		
		resultDiv.addEventListener("dblclick", this.clearconsole, false);

		setspeed_TA.addEventListener("change", this.sendData_setspeed_TA, false);
		setspeed_TB.addEventListener("change", this.sendData_setspeed_TB, false);
		setspeed_TC.addEventListener("change", this.sendData_setspeed_TC, false);
		
		
		scrolltopbtn1.addEventListener("touchstart", this.scrollTop, false);
		scrolltopbtn2.addEventListener("touchstart", this.scrollTop, false);
		scrolltopbtn3.addEventListener("touchstart", this.scrollTop, false);
		
		/*
		setstatus_T1_1.addEventListener("touchstart", this.sendData_setstatus_T1_1, false);
		setstatus_T1_0.addEventListener("touchstart", this.sendData_setstatus_T1_0, false);
		setstatus_T1_2.addEventListener("touchstart", this.sendData_setstatus_T1_2, false);
		
		setstatus_T2_1.addEventListener("touchstart", this.sendData_setstatus_T2_1, false);
		setstatus_T2_0.addEventListener("touchstart", this.sendData_setstatus_T2_0, false);
		setstatus_T2_2.addEventListener("touchstart", this.sendData_setstatus_T2_2, false);
		*/	
						
		setstatus_S1_0.addEventListener("touchstart", this.sendData_setstatus_S1_0, false);
		setstatus_S1_1.addEventListener("touchstart", this.sendData_setstatus_S1_1, false);
		
		setstatus_S2_0.addEventListener("touchstart", this.sendData_setstatus_S2_0, false);	
		setstatus_S2_1.addEventListener("touchstart", this.sendData_setstatus_S2_1, false);	
		
		setstatus_L1_0.addEventListener("touchstart", this.sendData_setstatus_L1_0, false);	
		setstatus_L1_1.addEventListener("touchstart", this.sendData_setstatus_L1_1, false);	
		
		setstatus_TA_1.addEventListener("touchstart", this.sendData_setstatus_TA_1, false);	
		setstatus_TA_0.addEventListener("touchstart", this.sendData_setstatus_TA_0, false);	
		setstatus_TB_1.addEventListener("touchstart", this.sendData_setstatus_TB_1, false);	
		setstatus_TB_0.addEventListener("touchstart", this.sendData_setstatus_TB_0, false);	
		setstatus_TC_1.addEventListener("touchstart", this.sendData_setstatus_TC_1, false);	
		setstatus_TC_0.addEventListener("touchstart", this.sendData_setstatus_TC_0, false);	
		
		setstatus_SS.addEventListener("touchstart", this.sendData_setstatus_SS, false);	
		setstatus_PA.addEventListener("touchstart", this.sendData_setstatus_PA, false);	
		setstatus_VI.addEventListener("touchstart", this.sendData_setstatus_VI, false);	
		setstatus_SR.addEventListener("touchstart", this.sendData_setstatus_SR, false);	
		setstatus_LC.addEventListener("touchstart", this.sendData_setstatus_LC, false);	

		
        disconnectButton.addEventListener(TOUCH_START, this.disconnect, false);
        deviceList.addEventListener('touchstart', this.connect, false);
    },
    onDeviceReady: function() {
        app.refreshDeviceList();
    },
    refreshDeviceList: function() {
        bluetoothSerial.list(app.onDeviceList, app.onError);
    },
    onDeviceList: function(devices) {
        var option;

        // remove existing devices
        deviceList.innerHTML = "";
        app.setStatus("");

        devices.forEach(function(device) {

            var listItem = document.createElement('li'),
                html = '<b>' + device.name + '</b><br/>' + device.id;

            listItem.innerHTML = html;

            if (cordova.platformId === 'windowsphone') {
              // This is a temporary hack until I get the list tap working
              var button = document.createElement('button');
              button.innerHTML = "Connesso";
              button.addEventListener('click', app.connect, false);
              button.dataset = {};
              button.dataset.deviceId = device.id;
              listItem.appendChild(button);
            } else {
              listItem.dataset.deviceId = device.id;
            }
            deviceList.appendChild(listItem);
        });

        if (devices.length === 0) {

            option = document.createElement('option');
            option.innerHTML = "Nessuna periferica Bluetooth trovata";
            deviceList.appendChild(option);

            if (cordova.platformId === "ios") { // BLE
                app.setStatus("Nessuna periferica Bluetooth trovata.");
            } else { // Android or Windows Phone
                app.setStatus("Associa un periferica Bluetooth.");
            }

        } else {
            app.setStatus("Trovat" + (devices.length === 1 ? "o " : "i ") + devices.length + " dispositiv" + (devices.length === 1 ? "o." : "i."));
        }

    },
    connect: function(e) {
        var onConnect = function() {
                // subscribe for incoming data
                bluetoothSerial.subscribe('\n', app.onData, app.onError);

                resultDiv.innerHTML="Inizio log:<br/>";
                app.setStatus("Connesso");
                app.showDetailPage();
            };

        var deviceId = e.target.dataset.deviceId;
        if (!deviceId) { // try the parent
            deviceId = e.target.parentNode.dataset.deviceId;
        }

        bluetoothSerial.connect(deviceId, onConnect, app.onError);
    },
    onData: function(data) { // data received from Arduino
	
		var currentdate = new Date(); 
		var datetime =  (currentdate.getDate()<10?'0':'') + currentdate.getDate() + "/"
						+ ((currentdate.getMonth()+1)<10?'0':'') + (currentdate.getMonth()+1)  + "/" 
						+ currentdate.getFullYear() + " @ "  
						+ (currentdate.getHours()<10?'0':'') + currentdate.getHours() + ":"  
						+ (currentdate.getMinutes()<10?'0':'') + currentdate.getMinutes() + ":" 
						+ (currentdate.getSeconds()<10?'0':'') + currentdate.getSeconds();
	    
		// todo se json setta stato treni e scambi
		if (data.charAt(0)=="{"){
			console.log(data);
		}
		
		resultDiv.innerHTML = resultDiv.innerHTML + "" +  datetime + "<br/>" +  "-->" + data + "<br/>";			
    },
	clearconsole: function(event) { 
		resultDiv.innerHTML="Inizio log:<br/>";
	},
	scrollTop: function(){
		scrollToTop(300);				
        resultDiv.scrollTop = resultDiv.scrollHeight;
	},
    sendData: function(event) { // send data to Arduino
	
		var currentdate = new Date(); 
		var datetime =  (currentdate.getDate()<10?'0':'') + currentdate.getDate() + "/"
						+ ((currentdate.getMonth()+1)<10?'0':'') + (currentdate.getMonth()+1)  + "/" 
						+ currentdate.getFullYear() + " @ "  
						+ (currentdate.getHours()<10?'0':'') + currentdate.getHours() + ":"  
						+ (currentdate.getMinutes()<10?'0':'') + currentdate.getMinutes() + ":" 
						+ (currentdate.getSeconds()<10?'0':'') + currentdate.getSeconds();		

        var success = function() {            
            resultDiv.innerHTML = resultDiv.innerHTML + "m9lab>" + datetime + "<br/>" + "Mandato: " +  messageInput.value + "<br/>";			
            //resultDiv.scrollTop = resultDiv.scrollHeight;
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = messageInput.value;
        bluetoothSerial.write(data, success, failure);
    },    
    sendData_setspeed_TA: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setspeed_TA.value;
        bluetoothSerial.write(data, success, failure);
    },
	sendData_setspeed_TB: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setspeed_TB.value;
        bluetoothSerial.write(data, success, failure);
    },	
	sendData_setspeed_TC: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setspeed_TC.value;
        bluetoothSerial.write(data, success, failure);
    },		
	sendData_setstatus_S1_1: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_S1_1.value;
        bluetoothSerial.write(data, success, failure);
    },	
	sendData_setstatus_S1_0: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_S1_0.value;
        bluetoothSerial.write(data, success, failure);
    },		
	sendData_setstatus_S2_1: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_S2_1.value;
        bluetoothSerial.write(data, success, failure);
    },	
	sendData_setstatus_S2_0: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_S2_0.value;
        bluetoothSerial.write(data, success, failure);
    },		
	sendData_setstatus_L1_0: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_L1_0.value;
        bluetoothSerial.write(data, success, failure);
    },	
	sendData_setstatus_L1_1: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_L1_1.value;
        bluetoothSerial.write(data, success, failure);
    },	
	sendData_setstatus_TA_0: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_TA_0.value;
        bluetoothSerial.write(data, success, failure);
    },	
	sendData_setstatus_TA_1: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");			
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_TA_1.value;
        bluetoothSerial.write(data, success, failure);
    },		
	sendData_setstatus_TB_0: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_TB_0.value;
        bluetoothSerial.write(data, success, failure);
    },	
	sendData_setstatus_TB_1: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_TB_1.value;
        bluetoothSerial.write(data, success, failure);
    },		
	sendData_setstatus_TC_1: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_TC_1.value;
        bluetoothSerial.write(data, success, failure);
    },	
	sendData_setstatus_TC_0: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_TC_0.value;
        bluetoothSerial.write(data, success, failure);
    },		
	sendData_setstatus_SS: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_SS.value;
        bluetoothSerial.write(data, success, failure);
    },	
	sendData_setstatus_SR: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_SR.value;
        bluetoothSerial.write(data, success, failure);
    },	
	sendData_setstatus_LC: function(event) { // send data to Arduino

        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_LC.value;
        bluetoothSerial.write(data, success, failure);
    },		
	sendData_setstatus_PA: function(event) { // send data to Arduino		
        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_PA.value;
        bluetoothSerial.write(data, success, failure);
    },	
	sendData_setstatus_VI: function(event) { // send data to Arduino		
        var success = function() {
            console.log("success");
        };

        var failure = function() {
            alert("Errore durante la scrittura su periferica Bluetooth");
        };

        var data = setstatus_VI.value;
        bluetoothSerial.write(data, success, failure);
    },		
    disconnect: function(event) {
        bluetoothSerial.disconnect(app.showMainPage, app.onError);
    },
    showMainPage: function() {
        mainPage.style.display = "";
        detailPage.style.display = "none";
    },
    showDetailPage: function() {
        mainPage.style.display = "none";
        detailPage.style.display = "";
    },
    setStatus: function(message) {
        console.log(message);

        window.clearTimeout(app.statusTimeout);
        statusDiv.innerHTML = message;
        statusDiv.className = 'fadein';

        // automatically clear the status with a timer
        app.statusTimeout = setTimeout(function () {
            statusDiv.className = 'fadeout';
        }, 5000);
    },
    onError: function(reason) {
        alert("ERRORE: " + reason); // real apps should use notification.alert
    }
};




function scrollToTop(scrollDuration) {
var scrollStep = -window.scrollY / (scrollDuration / 15),
	scrollInterval = setInterval(function(){
	if ( window.scrollY != 0 ) {
		window.scrollBy( 0, scrollStep );
	}
	else clearInterval(scrollInterval); 
},15);
}