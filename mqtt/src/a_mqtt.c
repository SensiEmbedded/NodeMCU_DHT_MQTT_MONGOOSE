#include <math.h>
#include "mgos.h"
#include "mgos_mqtt.h"

const char* publishTopic = "askoOut";

int led = 2;



void SaveNewHistValue(float val){
  //todo:
  (void)val;

}

void MqttPublishMeasuredDataIfNeeded(float temp, float hum){
  static float hum_last;
  char topic[100];

  if( fabs(hum_last - hum) > currentHumidityHysteresis){

    bool res = mgos_mqtt_pubf(topic, 0, false /* retain */,
                            "{hum: %fl, temp: %fl}",
                            hum,
                            temp);
    hum_last = hum;                            
    LOG(LL_INFO, ("measured data published: %s", res ? "yes" : "no"));                        
  }

}

boo MqttPublishMeasuredDataIfNeeded(char * mess){

  bool res = mgos_mqtt_pubf(topic, 0, false /* retain */,  mess);
  LOG(LL_INFO, ("measured data published: %s", res ? "yes" : "no"));                        
  return res;
}


static void gpio_int_handler(int pin, void *arg) {
  char topic[100];

  //snprintf(topic, sizeof(topic), "askoOut");
  //LOG(LL_INFO,(topic));         
  mgos_gpio_toggle(led);
  bool res = mgos_mqtt_pubf(publishTopic, 0, false /* retain */,
                            "{total_ram: %lu, free_ram: %lu}",
                            (unsigned long) mgos_get_heap_size(),
                            (unsigned long) mgos_get_free_heap_size());
  char buf[8];
  LOG(LL_INFO, ("Pin: %s, published: %s", mgos_gpio_str(pin, buf), res ? "yes" : "no"));
  (void) arg;
}  


static void mqtt_subscribe_cb(struct mg_connection *nc, int ev,
                                 void *ev_data, void *user_data)
{
    struct mg_mqtt_message *msg = (struct mg_mqtt_message *) ev_data;
    struct mg_str *payload = &msg->payload;
    char *command;
    float val;

    if (ev != MG_EV_MQTT_PUBLISH)
        return;

    if (json_scanf(payload->p, payload->len, "{command: %Q}", &command) != 1) {
        LOG(LL_ERROR, ("Malformed control input"));
        return;
    }
    LOG(LL_INFO, ("Received command '%s'", command));
    if (strcmp(command, "reboot") == 0) {
        LOG(LL_WARN, ("Rebooting"));
        mgos_system_restart_after(100);
    }
    if (strcmp(command, "SetHumHysteresis") == 0) {
        //string of type {command:"SetHumHysteresis",value:"123.2"}
        LOG(LL_INFO, ("Set Humidity Hysteresis command came:"));
        if (json_scanf(payload->p, payload->len, "{value: %f}", &val) != 1) {
          LOG(LL_ERROR, ("Cannot extract value of hystresis."));
          return;
        } 
        LOG(LL_INFO,("new value for hysteresis is:%f",val));
        SaveNewHistValue(val);
    }
    (void)user_data;
    (void)nc;
    free(command);
}

static void ev_handler(struct mg_connection *c, int ev, void *p,
                       void *user_data) {

  

  struct mg_mqtt_message *msg = (struct mg_mqtt_message *) p;

  if (ev == MG_EV_MQTT_CONNACK) {
    LOG(LL_INFO, ("CONNACK: %d", msg->connack_ret_code));
    if (mgos_sys_config_get_mqtt_sub() == NULL ||
        mgos_sys_config_get_mqtt_pub() == NULL) {
      LOG(LL_ERROR, ("Run 'mgos config-set mqtt.sub=... mqtt.pub=...'"));
    } else {
      mgos_mqtt_global_subscribe(mg_mk_str("askotest"), mqtt_subscribe_cb, NULL); 
      //./sub(c, "%s", mgos_sys_config_get_mqtt_sub());
    }
  } else if (ev == MG_EV_MQTT_SUBACK) {
    LOG(LL_INFO, ("Subscription %u acknowledged", msg->message_id));
  } else if (ev == MG_EV_MQTT_PUBLISH) {
    struct mg_str *s = &msg->payload;
    //struct json_token t = JSON_INVALID_TOKEN;
    //char buf[100], asciibuf[sizeof(buf) * 2 + 1];
    

    LOG(LL_INFO, ("got command: [%.*s]", (int) s->len, s->p));
    // Our subscription is at QoS 1, we must acknowledge messages sent to us. 
    mg_mqtt_puback(c, msg->message_id);
    
    /*if (json_scanf(s->p, s->len, "{gpio: {pin: %d, state: %d}}", &pin,&state) == 2) {
      int pin, state;
      // Set GPIO pin to a given state 
      mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_OUTPUT);
      mgos_gpio_write(pin, (state > 0 ? 1 : 0));
      pub(c, "{type: %Q, pin: %d, state: %d}", "gpio", pin, state);
    } else {
      pub(c, "{error: {code: %d, message: %Q}}", ERROR_UNKNOWN_COMMAND,
          "unknown command");
    }*/
  }
  (void) user_data;
  //(void)pub;
  //(void)sub;
}

//void MqqtPublishMeasureData(char* message){

//}
static void adc_cb(void *arg) {

  (void)arg;
}  

void MQTTInstallEventHandler(){
  int pin = 0;
  mgos_gpio_set_button_handler(pin, MGOS_GPIO_PULL_UP,
                                   MGOS_GPIO_INT_EDGE_POS, 50, gpio_int_handler,
                                   NULL);
  mgos_gpio_set_mode(led,MGOS_GPIO_MODE_OUTPUT);

  //./mgos_mqtt_global_subscribe(mg_mk_str("askotest"), mqtt_control_handler, NULL);

  mgos_mqtt_add_global_handler(ev_handler, NULL);

  mgos_set_timer(1000 /* ms */, true /* repeat */, adc_cb, NULL);  

}