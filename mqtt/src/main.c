
#include "mgos.h"
#include "mgos_rpc.h"

#include "a_mqtt.h"
#include "a_dht.h"



enum mgos_app_init_result mgos_app_init(void) {
  LOG(LL_INFO, ("Starting...\n"));

  int r = DHTInstallEventHandlerTimer();
  if ( r > 0 ) return r;

  MQTTInstallEventHandler();

  return MGOS_APP_INIT_SUCCESS;
}
