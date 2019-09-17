import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { BluetoothSerial } from '@ionic-native/bluetooth-serial';
import { AlertController, ToastController } from 'ionic-angular';


@Component({
  selector: 'page-home',
  templateUrl: 'home.html'
})

export class HomePage {

  pairedList: pairedlist;
  listToggle: boolean = false;
  pairedDeviceID: number = 0;
  dataSend: string = "";
  connected =false;

  allinput:any;


  // default value
  trackA_speed = 170;
  trackB_speed = 170;
  trackC_speed = 170;
  TA=0;
  TB=0;
  TC=0;
  S1=0;
  S2=0;


  constructor(public navCtrl: NavController, private alertCtrl: AlertController, private bluetoothSerial: BluetoothSerial, private toastCtrl: ToastController) {
    this.checkBluetoothEnabled();    
  }
 

  checkBluetoothEnabled() {
    this.bluetoothSerial.isEnabled().then(success => {
      this.listPairedDevices();
      this.pairedDeviceID = JSON.parse(localStorage.getItem('treninobt_address'));
    }, error => {
      this.showError("Please Enable Bluetooth")
    });
  }

  listPairedDevices() {
    this.bluetoothSerial.list().then(success => {
      this.pairedList = success;
      this.listToggle = true;
    }, error => {
      this.showError("Please Enable Bluetooth")
      this.listToggle = false;
    });
  }

  selectDevice() {
    let connectedDevice = this.pairedList[this.pairedDeviceID];
    if (!connectedDevice.address) {
      this.showError('Select Paired Device to connect');
      return;
    }

    let address = connectedDevice.address;
    let name = connectedDevice.name;
          
    this.connect(address,name);
  }

  connect(address,name) {
    // Attempt to connect device with specified address, call app.deviceConnected if success
    this.bluetoothSerial.connect(address).subscribe(success => {
      this.deviceConnected();
      this.showToast("Successfully Connected to " + name );    
      if (! this.connected) this.systemStatus();
      this.connected =true;  
      localStorage.setItem('treninobt_address',JSON.stringify(this.pairedDeviceID));  
              
    }, error => {
      this.showError("Error: Connecting to Device");
    });
  }

  deviceConnected() {
    // Subscribe to data receiving as soon as the delimiter is read
    this.bluetoothSerial.subscribe('\n').subscribe(success => {    
      this.handleData(success);
      //this.showToast("Connected Successfullly");      
    }, error => {
      this.showError(error);
    });
  }

  deviceDisconnect() {
    // Unsubscribe from data receiving
    this.bluetoothSerial.disconnect();
    this.connected =false;
    this.showToast("Device Disconnected");
  }

  handleData(data) {     
    //this.showToast(data);     
    try {      
      this.setAllValue(JSON.parse(data));  
    }
    catch(error) {
      //this.showError(error);
      console.log(error);      
    }
  }
  
  sendData(input:string) {  
        
    this.dataSend = input;
    this.dataSend+='\n';    
    
    this.bluetoothSerial.write(this.dataSend).then(success => {        
      this.showToast('Command sent');                             
    }, error => {
      this.showError(error)
    });
  }

  showError(error) {
    let alert = this.alertCtrl.create({
      title: 'Error',
      subTitle: error,
      buttons: ['Dismiss']
    });
    alert.present();
  }

  showToast(msj) {
    const toast = this.toastCtrl.create({
      message: msj,
      duration: 1000
    });
    toast.present();

  }


  setSpeed(track:string,vel:string){
    this.sendData("v"+track+"|"+vel);
  }

  systemStatus(){
    this.sendData("ss|0");
  }

  setAllValue(data){      
    
    //console.log(data);
    this.trackA_speed  = data.SA;
    this.trackB_speed = data.SB;
    this.trackC_speed = data.SC;
    this.TA = data.TA;
    this.TB = data.TB;        
    this.TC = data.TC;        
    this.S1 = data.S1; 
    this.S2 = data.S2; 
  }


  setTime(){
    //st|ore:minuti
    let date = new Date()    
    this.sendData("st|" + date.getHours() + ":" + date.getMinutes());
  }

}

interface pairedlist {
  "class": number,
  "id": string,
  "address": string,
  "name": string
}
