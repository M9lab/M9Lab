import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { BluetoothSerial } from '@ionic-native/bluetooth-serial';
import { AlertController, ToastController, LoadingController } from 'ionic-angular';

@Component({
  selector: 'page-home',
  templateUrl: 'home.html'
})

export class HomePage {

  pairedList: any;
  savedPairedList: any=[];
  listToggle: boolean = false;
  ready=false

  dataSend: string = "";
  selectedDevicesList: any=[];

  // default value
  track_speed = 170;
  track_status=0;

  loading:any

  /* to test in browser */
  isSimulated:boolean=false

  deviceType:any = [
    {"name": "Motor" , "value": "motor" , "selected": true, "disabled": false},
    {"name": "Switch" , "value": "switch" , "selected": false, "disabled": true},
    {"name": "Sensor" , "value": "sensor" , "selected": false, "disabled": true},
    {"name": "Light" , "value": "light" , "selected": false, "disabled": true}
  ] ;   

  constructor(public navCtrl: NavController, private alertCtrl: AlertController, private bluetoothSerial: BluetoothSerial, private toastCtrl: ToastController, public loadingCtrl: LoadingController) {        
    this.checkBluetoothEnabled();        
  }

  presentLoadingDefault() {
    this.loading = this.loadingCtrl.create({
      content: 'Please wait...'
    });
  
    this.loading.present();
  }  

  dismissLoadingDefault() {
    this.loading.dismiss();
  }
 

  checkBluetoothEnabled() {

    if (this.isSimulated){
      this.listPairedDevicesFake();
      return;
    }

    this.bluetoothSerial.isEnabled().then(success => {
      this.listPairedDevices();      
    }, error => {
      this.showError("Please Enable Bluetooth")
    });
  }

  listPairedDevicesFake() {

    this.pairedList = [
      {"class": 7936 , "id": "98:D3:32:30:5C:8F" , "address": "98:D3:32:30:5C:8F", "name": "M9lab01"},
      {"class": 7936, "id": "98:D3:32:10:DB:79", "address": "98:D3:32:10:DB:79", "name": "NiVo001"},
      {"class": 7936, "id": "AA:D3:32:11:DB:70", "address": "AA:D3:32:11:DB:70", "name": "NiVo002"}
    ];

     
    if (typeof(localStorage.getItem('NiVo_lastSelected'))!="undefined") this.savedPairedList = JSON.parse(localStorage.getItem('NiVo_lastSelected')) 
      
    this.parsePairedList()
    this.listToggle = true;

  }

  listPairedDevices() {

    if(this.isSimulated){
      this.listPairedDevicesFake()
      return;
    }
        
    this.bluetoothSerial.list().then(success => {      
      this.pairedList = success;
         
      if (typeof(localStorage.getItem('NiVo_lastSelected'))!="undefined") this.savedPairedList = JSON.parse(localStorage.getItem('NiVo_lastSelected')) 
      
      this.parsePairedList()
      this.listToggle = true;
      
    }, error => {
      this.showError("Please Enable Bluetooth")
      this.listToggle = false;
    });

  }
  

  parsePairedList(){
    this.selectedDevicesList=[];
    this.pairedList.forEach(element => {      
      if (this.savedPairedList==null){
        element["checked"] = false
        //element['type'] = "motor";
        delete element["checked"];
      }else{

        let savedDevice = this.savedPairedList.filter(x => x.address == element["address"])[0];              

        if (typeof(savedDevice)!="undefined"){                
          element['type'] = savedDevice['type']
          element["checked"] = true        
          delete element["id"];
          delete element["class"];   
          this.selectedDevicesList.push(element)
        }else{
          element["checked"] = false
          //element['type'] = "motor";
          delete element["checked"];        
        }  
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
            this.presentLoadingDefault();     
            if(element["connected"]){

              
              if (this.isSimulated){
                element["connected"]=false;
                this.dismissLoadingDefault();
                return;
              }
              
              this.deviceDisconnect().then((res)=>{
                if (res) element["connected"]=false;
                this.dismissLoadingDefault();
              })
            }else{

              if (this.isSimulated){
                element["connected"]=true;
                this.dismissLoadingDefault();
                return;
              }
              
                       
              this.deviceDisconnect().then(()=>{
                this.connect(device.address,device.name).then((res)=>{
                  if (res) element["connected"]=true
                    this.dismissLoadingDefault();
                    
                })
              })                            
              
            }
            return;
        }else{
          //this.dismissLoadingDefault();
          element["connected"]=false;
        } 

      })             

  }

  connect(address,name) {

    // Attempt to connect device with specified address, call app.deviceConnected if success

    return new Promise((resolve, reject) =>{

      this.bluetoothSerial.connect(address).subscribe(success => {
        this.deviceConnected();
        this.showToast("Successfully Connected to " + name );    
        
        // ok connect
        this.selectedDevicesList.forEach(element => {
          if (element["address"] == address){
            element["connected"]=true;
            this.systemStatus();
          }
        })      
        resolve(true)
                
      }, error => {
        this.showError("Error:Connecting to Device");
        resolve(false)
      });

    })  
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
      //this.showToast("Device Disconnected");
      resolve(true);
    })  

  }

  returnToList(){
    this.deviceDisconnect().then(()=>{
      this.selectedDevicesList.forEach(element => {
        element["connected"]=false; 
      })
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
