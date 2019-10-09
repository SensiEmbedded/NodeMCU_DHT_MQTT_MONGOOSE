#include "mgos.h"
#include "mgos_dht.h"
#include "a_dht.h"
#include "a_mqtt.h"
#include "mgos_rpc.h"
#include <math.h>

static struct mgos_dht *s_dht = NULL;
static int led = 2;
bool blink = false;
float currentHumidityHysteresis = 0;


static void CheckMeasurementsForSignificantChange(float hum, float temp){
  static int first = 0;
  static float hum_last;
  char mess[50];
  if(first == 0){
    first = 1;
    hum_last = -1000;//force to publish 
  }

  if( fabs(hum_last - hum) > currentHumidityHysteresis){
    sprintf(mess, "hum = %f, temp = %f",hum,temp);
    if( MqttPublishMeasuredDataIfNeeded(mess) == true){
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
  CheckMeasurementsForSignificantChange(h,t);

  //LOG(LL_INFO, ("Temp: %f *C Humidity: %f %%\n", t, h));
  (void) arg;
}
static void rpc_cb(struct mg_rpc_request_info *ri, void *cb_arg,
                   struct mg_rpc_frame_info *fi, struct mg_str args) {
  //float t = mgos_dht_get_temp(cb_arg);
  (void)cb_arg;
  float t = mgos_dht_get_temp(s_dht);
  float h = mgos_dht_get_humidity(s_dht);
  
  

  mg_rpc_send_responsef(ri, "{temp: %lf, hum: %lf}",t,h);
  //mg_rpc_send_responsef(ri, "{temp: %lf; hum: %lf}", mgos_dht_get_temp(cb_arg),mgos_dht_get_humidity(cb_arg)));
  (void) fi;
  (void) args;
}
int DHTInstallEventHandlerTimer(){
  (void)led;
  //./mgos_gpio_set_mode(led,MGOS_GPIO_MODE_OUTPUT);
  //if ((s_dht = mgos_dht_create(16, DHT11)) == NULL){ 
  //pin 14 == D5; pin 16 D0  
  if ((s_dht = mgos_dht_create(14, DHT11)) == NULL){ 
    LOG(LL_INFO, ("Error initializing DHT lib\n"));
    return MGOS_APP_INIT_ERROR;
  }
  mg_rpc_add_handler(mgos_rpc_get_global(), "Temp.Read", "", rpc_cb, s_dht);
  mgos_set_timer(2000 /* ms */, true /* repeat */, dht_timer_cb, NULL);  
  
  return 0;
}  

