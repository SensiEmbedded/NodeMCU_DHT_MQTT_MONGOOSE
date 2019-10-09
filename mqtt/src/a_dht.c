#include "mgos.h"
#include "mgos_dht.h"
#include "a_dht.h"
#include "a_mqtt.h"
#include "mgos_rpc.h"
#include <math.h>

int led = 2;

static struct mgos_dht *s_dht = NULL;
bool blink = false;
float currentHumidityHysteresis = 0;
float hum, temp;
bool forcePublishData = false;


void SaveNewHistValue(float val){
  char mess[50];
  //todo: Save to global config
  if( currentHumidityHysteresis != val){
    // Hysteresis in config_schema: is:
    //- ["hysteresis", "i", 0, {title: "Some integer value "}]
    // because I cannot find in documentation how to declare float value
    // 
    //mgos_config_apply("{\"hysteresis\": 17}", true);//this is working
    sprintf(mess,"{\"hysteresis\": %d}",(int)val);
    mgos_config_apply(mess, true);
  }

  currentHumidityHysteresis = val;

}

static void CheckMeasurementsForSignificantChange(float hum, float temp,bool forcePublish){
  static int first = 0;
  static float hum_last;
  char mess[50];
  if(first == 0){
    first = 1;
    hum_last = -1000;//force to publish 
  }

  //LOG(LL_INFO, ("hum_last = %f",hum_last));

  if( (fabs(hum_last - hum) > currentHumidityHysteresis )||( forcePublish == true)){
    sprintf(mess, "hum = %f, temp = %f,hyst = %f",hum,temp,currentHumidityHysteresis);
    LOG(LL_INFO, (mess));
    if( MqttPublishMeasuredData(mess) == true){
      hum_last = hum;
    }
  }
}

static void dht_timer_cb(void *arg) {
  blink = !blink;
  if(blink){
    //LOG(LL_INFO, ("Blink 1\n"));
  }else{
    //LOG(LL_INFO, ("Blink 0\n"));
  }
  //mgos_gpio_write(led,blink);
  
  float t = mgos_dht_get_temp(s_dht);
  float h = mgos_dht_get_humidity(s_dht);

  if (isnan(h) || isnan(t)) {
    LOG(LL_INFO, ("Failed to read data from sensor\n"));
    return;
  }
  hum = h;//set global variables
  temp = t;
  CheckMeasurementsForSignificantChange(h,t,forcePublishData /*forcePublish*/);
  if(forcePublishData == true){
    forcePublishData = false;
  }
  
  //LOG(LL_INFO, ("Temp: %f *C Humidity: %f %%\n", t, h));
  (void) arg;
}
static void rpc_cb(struct mg_rpc_request_info *ri, void *cb_arg,
                   struct mg_rpc_frame_info *fi, struct mg_str args) {
  (void)cb_arg;
  float t = mgos_dht_get_temp(s_dht);
  float h = mgos_dht_get_humidity(s_dht);
  mg_rpc_send_responsef(ri, "{temp: %lf, hum: %lf}",t,h);
  //mg_rpc_send_responsef(ri, "{temp: %lf; hum: %lf}", mgos_dht_get_temp(cb_arg),mgos_dht_get_humidity(cb_arg)));
  (void) fi;
  (void) args;
}

static void gpio_int_handler(int pin, void *arg) {
  char mess[50];
  CheckMeasurementsForSignificantChange(hum,temp,false /*forcePublish*/);

  sprintf(mess, "hum = %f, temp = %f,hyst = %f",hum,temp,currentHumidityHysteresis);
  MqttPublishMeasuredData(mess);
  //snprintf(topic, sizeof(topic), "askoOut");
  //LOG(LL_INFO,(topic));         
  mgos_gpio_toggle(led);
  (void) arg;
  (void) pin;
}  

int DHTInstallEventHandlerTimer(){

  
  
  int h = mgos_sys_config_get_hysteresis();
  currentHumidityHysteresis = (float)h;
  LOG(LL_INFO, ("Read hyst from conf %d\n",h));


  //if ((s_dht = mgos_dht_create(16, DHT11)) == NULL){ 
  //pin 14 == D5; pin 16 D0  
  if ((s_dht = mgos_dht_create(14, DHT11)) == NULL){ 
    LOG(LL_INFO, ("Error initializing DHT lib\n"));
    return MGOS_APP_INIT_ERROR;
  }
  mg_rpc_add_handler(mgos_rpc_get_global(), "Temp.Read", "", rpc_cb, s_dht);
  mgos_set_timer(2000 /* ms */, true /* repeat */, dht_timer_cb, NULL);  

  mgos_gpio_set_mode(led,MGOS_GPIO_MODE_OUTPUT);

  int pin = 0;
  
  mgos_gpio_set_button_handler(pin, MGOS_GPIO_PULL_UP,
                                   MGOS_GPIO_INT_EDGE_POS, 50, gpio_int_handler,
                                   NULL);
  
  return 0;
}  

