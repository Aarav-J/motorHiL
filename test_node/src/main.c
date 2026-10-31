#include <zephyr/drivers/counter.h> 
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hil_test_node, LOG_LEVEL_INF);
#define TEST_INTERVAL_US 50000U
#define ALARM_CHAN_ID 0


static const struct device *const count_dev = DEVICE_DT_GET(DT_CHILD(DT_NODELABEL(timer0), counter)); 


static volatile uint32_t alarm_count = 0; 
static struct counter_alarm_cfg alarm_cfg; 


static void counter_alarm_callback(const struct device *dev, uint8_t chan_id, uint32_t ticks, void *user_data) { 

    alarm_count++; 
    counter_set_channel_alarm(dev, chan_id, &alarm_cfg); 

}


int main(void) {
    LOG_INF("HIL test node booting");
    if(!device_is_ready(count_dev)) { 
        LOG_ERR("Counter device not ready");
        return -1; 
    }
    counter_start(count_dev);
    alarm_cfg.callback = counter_alarm_callback;
    alarm_cfg.ticks = counter_us_to_ticks(count_dev, TEST_INTERVAL_US); 
    alarm_cfg.user_data = NULL; 
    alarm_cfg.flags = 0; 
    
    int err = counter_set_channel_alarm(count_dev, ALARM_CHAN_ID, &alarm_cfg); 
    if(err != 0) { 
        LOG_ERR("Failed to set alarm: %d", err); 
        return -1; 
    }
    while (1) {
        k_sleep(K_SECONDS(2));
        LOG_INF("heartbeat: uptime=%u ms", k_uptime_get_32());
        LOG_INF("alarm_count=%u", alarm_count);
    }

    return 0;
}
