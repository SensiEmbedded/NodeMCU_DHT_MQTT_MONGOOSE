// load('api_config.js');
// load('api_gpio.js');
// load('api_mqtt.js');
// load('api_net.js');
// load('api_sys.js');
// load('api_timer.js');

// //let led = Cfg.get('pins.led');
// //let button = Cfg.get('pins.button');
// //let topic = '/asko/' + Cfg.get('mqtt.client_id') + '/state';
// let led = 2;
// let button = 0;
// let topic = 'asko';

// //./print('LED GPIO:', led, 'button GPIO:', button);

// let getInfo = function() {
//   return JSON.stringify({
//     data: {
//       total_ram: Sys.total_ram(),
//       free_ram: Sys.free_ram()
//     }
//   });
// };

// // Blink built-in LED every second
// GPIO.set_mode(led, GPIO.MODE_OUTPUT);
// GPIO.setup_output(led, true);

// //function Publish(){
  
//   //./let message = getInfo();
//   //./let ok = MQTT.pub(topic, message, 1);
//   //./print('Published:', ok, topic, '->', message);
// //}

// Timer.set(1000 /* 1 sec */, true /* repeat */, function() {
//   //let value = GPIO.toggle(led);
//   //print(value ? 'Tick' : 'Tock', 'uptime:', Sys.uptime(), getInfo());
// }, null);

// // Publish to MQTT topic on a button press. Button is wired to GPIO pin 0
// GPIO.set_button_handler(button, GPIO.PULL_UP, GPIO.INT_EDGE_NEG, 200, function() {
//   GPIO.toggle(led);
//   let message = getInfo();
//   let ok = MQTT.pub(topic, message, 1);
//   print('Published:', ok, topic, '->', message);
// }, null)