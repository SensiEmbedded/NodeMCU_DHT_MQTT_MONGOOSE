#include <math.h>
#include "mgos.h"
#include "mgos_mqtt.h"
#include "a_dht.h"

const char* publishTopic = "askoOut";
const char* subcribeTopic = "askoIn";





bool MqttPublishMeasuredData(char * mess){

  bool res = mgos_mqtt_pubf(publishTopic, 0, false /* retain */,  mess);
  LOG(LL_INFO, ("measured data published: %s", res ? "yes" : "no"));                        
  return res;
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
    if (strcmp(command, "refresh") == 0) {
      //string of type {command:"refresh"}
        LOG(LL_INFO, ("Refresh command"));
        forcePublishData = true;
        return;
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
      mgos_mqtt_global_subscribe(mg_mk_str(subcribeTopic), mqtt_subscribe_cb, NULL); 
    }
  } else if (ev == MG_EV_MQTT_SUBACK) {
    LOG(LL_INFO, ("Subscription %u acknowledged", msg->message_id));
  } else if (ev == MG_EV_MQTT_PUBLISH) {
    struct mg_str *s = &msg->payload;
    LOG(LL_INFO, ("got command: [%.*s]", (int) s->len, s->p));
    // Our subscription is at QoS 1, we must acknowledge messages sent to us. 
    mg_mqtt_puback(c, msg->message_id);
  }
  (void) user_data;
}

static void adc_cb(void *arg) {

  (void)arg;
}  

void MQTTInstallEventHandler(){
  
  

  //./mgos_mqtt_global_subscribe(mg_mk_str("askotest"), mqtt_control_handler, NULL);

  mgos_mqtt_add_global_handler(ev_handler, NULL);

  mgos_set_timer(1000 /* ms */, true /* repeat */, adc_cb, NULL);  

}