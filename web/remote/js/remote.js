/* Moving Light Show remote control                          *
 * https://MovingLightShow.art - contact@movinglightshow.art *
 * Version 1.0.8.4                                          *
 * (c) 2020-2021 Showband Les Armourins                      */

const Version = "1.0.8.4";
const Copyright = "&copy; 2020-2021 Showband Les Armourins";

const UART_SERVICE_UUID = "fe150000-c76e-46b7-a964-3358a4efcf62";
const UART_RX_CHARACTERISTIC_UUID = "fe150001-c76e-46b7-a964-3358a4efcf62";
const UART_TX_CHARACTERISTIC_UUID = "fe150002-c76e-46b7-a964-3358a4efcf62";

var movingLightShowDevice = null;
var movingLightShowService;

var bleReceived = 0;
var lastBleReceivedTS = 0;

var cmdCounter = 0;
var lastCmd = "";
var lastCmdTS = 0;
var checkTimer;

var manualDisconnect = false;

var rxCharacteristic = null;

var networkPrefix = "AMX";

var bleConnected = false;

var crc8 = new CRC8();

/*
// Initialize deferredPrompt for use later to show browser install prompt.
let deferredPrompt;

window.addEventListener('beforeinstallprompt', (e) => {
  // Prevent the mini-infobar from appearing on mobile
  e.preventDefault();
  // Stash the event so it can be triggered later.
  deferredPrompt = e;
  // Update UI notify the user they can install the PWA
  showInstallPromotion();
  // Optionally, send analytics event that PWA install promo was shown.
  // console.log(`'beforeinstallprompt' event was fired.`);
});
*/

// Check also https://googlechrome.github.io/samples/web-bluetooth/automatic-reconnect.html

function displayBLEconnectInitial() {
  bleConnected = false;
  document.getElementById("ble").innerHTML = '<i class="fab fa-bluetooth has-text-grey"></i>&nbsp;Connect';
  // document.getElementById("connect-status").classList.add('disconnected-warning');
  document.getElementById("ble").classList.remove('is-warning'); // yellow
  document.getElementById("ble").classList.remove('is-info'); // blue
  document.getElementById("ble").classList.add('is-light'); // gray/light
}


function displayBLEconnect() {
  bleConnected = false;
  document.getElementById("ble").innerHTML = '<i class="fab fa-bluetooth has-text-grey"></i>&nbsp;Connect';
  // document.getElementById("connect-status").classList.add('disconnected-warning');
  document.getElementById("ble").classList.remove('is-light'); // gray/light
  document.getElementById("ble").classList.remove('is-info'); // blue
  document.getElementById("ble").classList.add('is-warning'); // yellow
}


function displayBLEconnecting() {
  bleConnected = false;
  document.getElementById("ble").innerHTML = '<i class="fab fa-bluetooth has-text-grey"></i>&nbsp;Connecting...';
  // document.getElementById("connect-status").classList.add('disconnected-warning');
  document.getElementById("ble").classList.remove('is-light'); // gray/light
  document.getElementById("ble").classList.remove('is-info'); // blue
  document.getElementById("ble").classList.add('is-warning'); // yellow
}


function displayBLEdisconnect() {
  bleConnected = true;
  document.getElementById("ble").innerHTML = '<i class="fab fa-bluetooth has-text-link"></i>&nbsp;Disconnect';
  // document.getElementById("connect-status").classList.remove('disconnected-warning');
  document.getElementById("ble").classList.remove('is-light'); // gray/light
  document.getElementById("ble").classList.remove('is-warning'); // yellow
  document.getElementById("ble").classList.add('is-info'); // blue
}


function init() {
  document.getElementById("copyright").innerHTML = Copyright + ' - Version ' + Version;
  checkLoop();
}


function BLEclick() {
  if (bleConnected) {
    disconnectBLEclick();
  } else {
    connectBLEclick();
  }
}


function connectBLEclick() {
  try {
    movingLightShowDevice = null;
    console.log("Requesting Bluetooth Device...");
    // document.getElementById("ble_state").className = "fas fa-times-circle has-text-danger";
    displayBLEconnectInitial();
    navigator.bluetooth.requestDevice({
      filters: [{ namePrefix: "MovingLightShow" }],
      optionalServices: [UART_SERVICE_UUID]
    })
    .then(device => {
      movingLightShowDevice = device;
      movingLightShowDevice.addEventListener('gattserverdisconnected', onDisconnectedBLE);
      // return connectBLE();
      displayBLEconnect();
      return movingLightShowDevice.gatt.connect();
    })
    .then(server => {
      console.log('Getting service...');
      return server.getPrimaryService(UART_SERVICE_UUID);
    })
    .then(service => {
      movingLightShowService = service;
      console.log('Getting characteristic...');
      return movingLightShowService.getCharacteristic(UART_TX_CHARACTERISTIC_UUID);
    })
    .then(characteristic => {
      characteristic.startNotifications();
      characteristic.addEventListener(
        "characteristicvaluechanged",
        onTxCharacteristicValueChanged
      );
      return movingLightShowService.getCharacteristic(UART_RX_CHARACTERISTIC_UUID);
    })
    .then(characteristic => {
      rxCharacteristic = characteristic;
      //document.getElementById("ble_state").className = "fab fa-bluetooth has-text-success";
      displayBLEdisconnect();
    })
    .catch(error => {
      // document.getElementById("ble_state").className = "fas fa-times-circle has-text-danger";
      displayBLEconnect();
      console.log('Argh! ' + error);
    });
  } catch (error) {
      console.log('Error! ' + error);
  }
}
    

function reconnectBLE() {
  // exponentialBackoff(6 /* max retries */, 1 /* seconds delay */,
  repeatTrials(30 /* max retries */, 1 /* seconds delay */,
    async function toTry() {
      console.log('Reconnecting to Bluetooth Device... ');
      // document.getElementById("ble_state").className = "fas fa-times-circle has-text-danger";
      displayBLEconnecting();
      if (movingLightShowDevice.gatt.connect()) {
        movingLightShowService = await movingLightShowDevice.gatt.getPrimaryService(UART_SERVICE_UUID);
        const characteristic = await movingLightShowService.getCharacteristic(UART_TX_CHARACTERISTIC_UUID);
        await characteristic.startNotifications();
        characteristic.addEventListener(
          "characteristicvaluechanged",
          onTxCharacteristicValueChanged
        );
        rxCharacteristic = await movingLightShowService.getCharacteristic(UART_RX_CHARACTERISTIC_UUID);
        if (rxCharacteristic) {
          // document.getElementById("ble_state").className = "fab fa-bluetooth has-text-success";
          displayBLEdisconnect();
          return true;
        }
      } else {
        return false;
      }
      
    },
    function success(server) {
      console.log('Bluetooth Device connected.');
      return server;
      /*
      const service = server.getPrimaryService(UART_SERVICE_UUID);
      const txCharacteristic = service.getCharacteristic(UART_TX_CHARACTERISTIC_UUID);
      txCharacteristic.startNotifications();
      txCharacteristic.addEventListener(
        "characteristicvaluechanged",
        onTxCharacteristicValueChanged
      );
      rxCharacteristic = service.getCharacteristic(UART_RX_CHARACTERISTIC_UUID);
      if (rxCharacteristic) {
        document.getElementById("ble_state").className = "fab fa-bluetooth has-text-success";
      } else {
        document.getElementById("ble_state").className = "fas fa-times-circle has-text-danger";
      }
      */
    },
    function fail() {
      displayBLEconnect();
      console.log('Failed to reconnect.');
    });
}


function onDisconnectedBLE() {
  console.log('Bluetooth Device disconnected');
  if (!manualDisconnect) {
    if (movingLightShowDevice) {
      reconnectBLE();
    }
  }
  manualDisconnect = false;
}


function disconnectBLEclick() {
  if (!movingLightShowDevice) {
    return;
  }

  if (movingLightShowDevice.gatt.connected) {
    manualDisconnect = true;
    movingLightShowDevice.gatt.disconnect();
    movingLightShowDevice = null;
    rxCharacteristic = null;
    // document.getElementById("ble_state").className = "fas fa-times-circle has-text-danger";
    displayBLEconnectInitial();
    console.log('Bluetooth Device manually disconnected');
  }
}


async function command(cmd) {
  cmdCounter++;
  console.log("Command " + cmd);
  if (!rxCharacteristic) {
    return;
  }
  try {
    let encoder = new TextEncoder();
    var data_array = encoder.encode(networkPrefix + cmd);
    var checksum = crc8.checksum(data_array);
    var mergedArray = new Uint8Array(data_array.length + 1);
    mergedArray.set(data_array);
    mergedArray[data_array.length] = checksum;
    rxCharacteristic.writeValue(mergedArray);
    let current_datetime = new Date();
    lastCmd = cmd;
    lastCmdTS = Math.round(current_datetime.getTime()/1000);
    // let formatted_date = current_datetime.getFullYear() + "-" + (current_datetime.getMonth() + 1) + "-" + current_datetime.getDate() + " " + current_datetime.getHours() + ":" + current_datetime.getMinutes() + ":" + current_datetime.getSeconds()
    // let formatted_date = current_datetime.getHours() + ":" + current_datetime.getMinutes() + ":" + current_datetime.getSeconds();
    // document.getElementById("info-right").innerHTML = ('[' +formatted_date + '] ' + cmd);
  } catch (error) {
    console.log(error);
  }
}


function checkLoop() {
  let current_datetime = new Date();
  let current_epoch = Math.round(current_datetime.getTime()/1000);
  if (current_epoch > (lastBleReceivedTS + 1)) {
    document.getElementById("info-pan").classList.remove('has-background-white');
    document.getElementById("info-pan").classList.add('has-background-warning');
  } else {
    document.getElementById("info-pan").classList.add('has-background-white');
    document.getElementById("info-pan").classList.remove('has-background-warning');
  }
  // clearTimeout(checkTimer);
  checkTimer = setTimeout(checkLoop, 1000);
}


// ble packet received
function onTxCharacteristicValueChanged(event) {
  let receivedData = [];
  let current_datetime = new Date();
  lastBleReceivedTS = Math.round(current_datetime.getTime()/1000);
  // let formatted_date = ("0" + current_datetime.getHours()).slice(-2) + ":" + ("0" + current_datetime.getMinutes()).slice(-2) + ":" + ("0" + current_datetime.getSeconds()).slice(-2);
  for (var i = 0; i < event.target.value.byteLength; i++) {
    receivedData[i] = event.target.value.getUint8(i);
  }
  const receivedString = String.fromCharCode.apply(null, receivedData) + ",";
  const info = receivedString.split(",");
  document.getElementById("info-left").innerHTML = info[0];
  // document.getElementById("info-right").innerHTML = formatted_date + " " + info[1];
  document.getElementById("info-right").innerHTML = info[1];
  bleReceived++;
}


// This function keeps calling "toTry" until promise resolves or has
// retried "max" number of times. First retry has a delay of "delay" seconds.
// "success" is called upon success.
function exponentialBackoff(max, delay, toTry, success, fail) {
  toTry().then(result => success(result))
  .catch(_ => {
    if (max === 0) {
      return fail();
    }
    console.log('Retrying in ' + delay + 's... (' + max + ' tries left)');
    setTimeout(function() {
      exponentialBackoff(--max, delay * 2, toTry, success, fail);
    }, delay * 1000);
  });
}


function repeatTrials(max, delay, toTry, success, fail) {
  toTry().then(result => success(result))
  .catch(_ => {
    if (max === 0) {
      return fail();
    }
    console.log('Retrying in ' + delay + 's... (' + max + ' tries left)');
    setTimeout(function() {
      repeatTrials(--max, delay, toTry, success, fail);
    }, delay * 1000);
  });
}
