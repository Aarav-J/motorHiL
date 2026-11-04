#include <zephyr/drivers/counter.h> 
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "drivers/quadrature.h"
LOG_MODULE_REGISTER(hil_test_node, LOG_LEVEL_INF);
#define TEST_INTERVAL_US 10000U
#define ALARM_CHAN_ID 0
#define SERVO_PIN 6
#define QUAD_PIN_A 4
#define QUAD_PIN_B 5
static volatile uint8_t quad_step = 0; 
static uint32_t period = 0; 
static uint32_t pulse_width = 0; 
static uint32_t counter_freq_hz = 0;
static const struct device *const count_dev = DEVICE_DT_GET(DT_CHILD(DT_NODELABEL(timer0), counter)); 
static const struct device *const gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));

static volatile uint32_t alarm_count = 0; 
static struct counter_alarm_cfg alarm_cfg; 
static struct gpio_callback servo_pwm_cfg; 


static void counter_alarm_callback(const struct device *dev, uint8_t chan_id, uint32_t ticks, void *user_data) { 

    alarm_count++; 
    quad_step = quadrature_advance_step(quad_step, QUADRATURE_DIR_FORWARD);
    quadrature_outputs_t outputs = quadrature_get_outputs(quad_step);
    gpio_pin_set(gpio_dev, QUAD_PIN_A, outputs.a);
    gpio_pin_set(gpio_dev, QUAD_PIN_B, outputs.b);
    counter_set_channel_alarm(dev, chan_id, &alarm_cfg); 



}

static void servo_pin_callback(const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins) { 
    static uint32_t last_rising_edge = 0; 
    uint32_t ticks = 0;
    if(gpio_pin_get(dev, SERVO_PIN)) { 
        counter_get_value(count_dev, &ticks); 
        uint32_t current_rising_edge = ticks;
        uint32_t period_ticks = current_rising_edge - last_rising_edge;
        period = (uint32_t)((uint64_t)period_ticks * 1000000ULL / counter_freq_hz);
        last_rising_edge = current_rising_edge;
    } else { 
        counter_get_value(count_dev, &ticks); 
        uint32_t current_falling_edge = ticks;
        uint32_t pulse_ticks = current_falling_edge - last_rising_edge;
        pulse_width = (uint32_t)((uint64_t)pulse_ticks * 1000000ULL / counter_freq_hz);
    }
}

int main(void) {
    LOG_INF("HIL test node booting");
    if(!device_is_ready(count_dev)) { 
        LOG_ERR("Counter device not ready");
        return -1; 
    }

    // Configuring GPIO as an output 
    if(!device_is_ready(gpio_dev)) { 
        LOG_ERR("GPIO device not ready");
        return -1; 
    }
    counter_freq_hz = counter_get_frequency(count_dev);
    gpio_pin_configure(gpio_dev, QUAD_PIN_A, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure(gpio_dev, QUAD_PIN_B, GPIO_OUTPUT_INACTIVE);
    int ret = gpio_pin_configure(gpio_dev, SERVO_PIN, GPIO_INPUT | GPIO_PULL_DOWN); 
    if(ret != 0) { 
        LOG_ERR("Failed to configure servo pin: %d", ret); 
        return -1; 
    }
    ret = gpio_pin_interrupt_configure(gpio_dev, SERVO_PIN, GPIO_INT_EDGE_BOTH);
    if(ret != 0) { 
        LOG_ERR("Failed to configure servo pin interrupt: %d", ret);
        return -1; 
    } 

    // Configure the GPIO callback for the servo pin
    gpio_init_callback(&servo_pwm_cfg, servo_pin_callback, BIT(SERVO_PIN));
    ret = gpio_add_callback(gpio_dev, &servo_pwm_cfg);
    if (ret != 0) {
        LOG_ERR("Failed to add servo pin callback: %d", ret);
        return -1;
    }

    counter_start(count_dev);
    //gpio_init_callback does not exist for counter, so we set the callback directly in the alarm configuration
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
    LOG_INF("Period: %u us, Pulse Width: %u us, duty: %u", period, pulse_width, (period > 0) ? (pulse_width * 100 / period) : 0);
    }

    return 0;
}
