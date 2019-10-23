import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { BluetoothSerial } from '@ionic-native/bluetooth-serial';
import { AlertController, ToastController } from 'ionic-angular';
//import { resolveReflectiveProviders } from '@angular/core/src/di/reflective_provider';

@Component({
  selector: 'page-home',
  templateUrl: 'home.html'
})

export class HomePage {

  pairedList: any;
  savedPairedList:any
  listToggle: boolean = false;
  ready=false

  dataSend: string = "";
  connected =false;

  selectedDevicesList: any=[];

  // default value
  track_speed = 170;
  track_status=0;

  deviceType:any = [
    {"name": "motor" , "value": "motor" , "selected": true, "disabled": false},
    {"name": "switch" , "value": "switch" , "selected": false, "disabled": true},
    {"name": "sensor" , "value": "sensor" , "selected": false, "disabled": true},
    {"name": "light" , "value": "light" , "selected": false, "disabled": true}
  ] ;   


  constructor(public navCtrl: NavController, private alertCtrl: AlertController, private bluetoothSerial: BluetoothSerial, private toastCtrl: ToastController) {
    this.checkBluetoothEnabled();
    //this.listPairedDevices();
  }
 

  checkBluetoothEnabled() {
    this.bluetoothSerial.isEnabled().then(success => {
      this.listPairedDevices();      
    }, error => {
      this.showError("Please Enable Bluetooth")
    });
  }

  listPairedDevices() {

    
    this.bluetoothSerial.list().then(success => {
      console.log(success)
      this.pairedList = success;

      // get saved data
      if (typeof(localStorage.getItem('NiVo_lastSelected'))!="undefined") this.savedPairedList = JSON.parse(localStorage.getItem('NiVo_lastSelected')) 
      
      this.parsePairedList()
      this.listToggle = true;
      
    }, error => {
      this.showError("Please Enable Bluetooth")
      this.listToggle = false;
    });
    

    /* test*/
    /*
    this.pairedList = [
      {"class": 7936 , "id": "98:D3:32:30:5C:8F" , "address": "98:D3:32:30:5C:8F", "name": "M9lab01"},
      {"class": 7936, "id": "98:D3:32:10:DB:79", "address": "98:D3:32:10:DB:79", "name": "NiVo001"},
      {"class": 7936, "id": "AA:D3:32:11:DB:70", "address": "AA:D3:32:11:DB:70", "name": "NiVo002"}
    ];

    // get saved data
    if (typeof(localStorage.getItem('NiVo_lastSelected'))!="undefined") this.savedPairedList = JSON.parse(localStorage.getItem('NiVo_lastSelected')) 
      
    this.parsePairedList()
    this.listToggle = true;


    */
       
  }
  

  parsePairedList(){
    this.selectedDevicesList=[];
    this.pairedList.forEach(element => {         
      let savedDevice = this.savedPairedList.filter(x => x.address == element["address"])[0];              
      if (typeof(savedDevice)!="undefined"){                
        element['type'] = savedDevice['type']
        element["checked"] = true        
        delete element["id"];
        delete element["class"];   
        this.selectedDevicesList.push(element)
      }else{
        element["checked"] = false
        element['type'] = "motor"
        delete element["checked"];        
      }            
    });
  }

  updateDevicesList(index,option){   
        
    if(option.checked){
      this.selectedDevicesList.push(option)
    }else{      
      let index = this.selectedDevicesList.indexOf(option);
      if (index !== -1) this.selectedDevicesList.splice(index, 1);
    }
  }

  selectDevices() {
    
    if (this.selectedDevicesList.length>0){
      localStorage.setItem('NiVo_lastSelected', JSON.stringify(this.selectedDevicesList))

      this.selectedDevicesList.forEach(element => {
        element["connected"] = false
      }) 

      this.ready =true;        
    }else{
      this.showError('Select at least one Paired Device to connect');      
    }
    
  }


  changeStatus(device){
          
      this.selectedDevicesList.forEach(element => {

        if (element["address"] == device.address){
            if(element["connected"]){
              element["connected"]=false;
              this.deviceDisconnect();
            }else{
              element["connected"]=true
                            
              this.deviceDisconnect().then(()=>{
                this.connect(device.address,device.name)
              })                            
              
            }
            return;
        }else{
          element["connected"]=false;
        } 

      })             

  }

  connect(address,name) {

    // Attempt to connect device with specified address, call app.deviceConnected if success
    this.bluetoothSerial.connect(address).subscribe(success => {
      this.deviceConnected();
      this.showToast("Successfully Connected to " + name );    
      if (! this.connected) this.systemStatus();
      this.connected =true;         
              
    }, error => {
      this.showError("Error:Connecting to Device");
    });
  }

  deviceConnected() {
    // Subscribe to data receiving as soon as the delimiter is read
    this.bluetoothSerial.subscribe('\n').subscribe(success => {    
      this.handleData(success);
      this.showToast("Connected Successfullly");      
    }, error => {
      this.showError(error);
    });
  }

  deviceDisconnect() {
    
    // Unsubscribe from data receiving
    return new Promise((resolve, reject) =>{

      this.bluetoothSerial.disconnect();    
      this.showToast("Device Disconnected");
      resolve(true);
    })  
  }

  returnToList(){
    this.deviceDisconnect().then(()=>{
      this.ready =false;
    })  
  }
 

  handleData(data) {     
      
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

    this.track_speed  = data.S1;    
    this.track_status = data.T1;
    //this.trackB_speed = data.S2;
    //this.T2 = data.T2;        
  }

}
