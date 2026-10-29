/*
 * HIL test node entry point -- scaffold only.
 *
 * This compiles and boots, logs a heartbeat, and nothing else yet. The
 * three real jobs (PWM duty capture, plant simulation + quadrature
 * generation, laptop command channel) are deliberately not implemented
 * here -- see prj.conf and boards/esp32s3_devkitc.overlay for the open
 * peripheral-choice decisions that need resolving first.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hil_test_node, LOG_LEVEL_INF);

/* Low-priority diagnostic thread -- separate from whatever ends up doing
 * the time-critical plant-update work, same split as the STM32 side's
 * ISR-does-minimal-work / main()-does-reporting principle. Fill in real
 * telemetry (simulated speed, fault state, last command received, etc.)
 * once the plant model exists. */
#define DIAG_STACK_SIZE 2048
#define DIAG_PRIORITY                                                          \
  10 /* low priority: only runs when nothing else is ready */
#define DIAG_PERIOD_MS 2000

K_THREAD_STACK_DEFINE(diag_stack, DIAG_STACK_SIZE);
static struct k_thread diag_thread_data;

static void diag_thread(void *p1, void *p2, void *p3) {
  ARG_UNUSED(p1);
  ARG_UNUSED(p2);
  ARG_UNUSED(p3);

  while (1) {
    k_sleep(K_MSEC(DIAG_PERIOD_MS));
    LOG_INF("heartbeat: uptime=%u ms", k_uptime_get_32());
  }
}

int main(void) {
  LOG_INF("HIL test node booting");

  k_thread_create(&diag_thread_data, diag_stack, DIAG_STACK_SIZE, diag_thread,
                  NULL, NULL, NULL, DIAG_PRIORITY, 0, K_NO_WAIT);

  return 0;
}
