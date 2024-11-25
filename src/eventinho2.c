/*
* Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * NOTE: If you are looking into an implementation of button events with
 * debouncing, check out `input` subsystem and `samples/subsys/input/input_dump`
 * example instead.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led2)
#define SW0_NODE    DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS_OKAY(SW0_NODE)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

//static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});

K_MUTEX_DEFINE(mutex_pisca);
K_MUTEX_DEFINE(mutex_evento);

void existe(void *p1, void *p2, void *p3){

    while(1){
        printk("ta existindo...");
    }
}

void pisca(void *p1, void *p2, void *p3){

    int ret1;
	ret1 = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);

        if(k_mutex_lock(&mutex_pisca, K_NO_WAIT) == 0){

           //k_mutex_lock(&mutex_pisca, K_NO_WAIT);
            gpio_pin_toggle_dt(&led0);
            printk("%d liberado e bloqueou", k_mutex_lock(&mutex_pisca, K_NO_WAIT));
            //k_msleep(5000);
            k_msleep(1000);
            //k_mutex_unlock(&mutex_pisca);
            //k_msleep(1000);
        }
            printk("%d bloqueado", k_mutex_lock(&mutex_pisca, K_NO_WAIT));

}

void button_pressed(const struct device *dev, struct gpio_callback *cb,
            uint32_t pins)
{
    printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

void evento(void *p1, void *p2, void *p3){
       
    static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios,
                             {0});

void button_pressed(const struct device *dev, struct gpio_callback *cb,
            uint32_t pins)
{
    printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}


    int ret;

    if (!gpio_is_ready_dt(&button)) {
        printk("Error: button device %s is not ready\n",
               button.port->name);

    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n",
               ret, button.port->name, button.pin);

    }

    ret = gpio_pin_interrupt_configure_dt(&button,
                          GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n",
            ret, button.port->name, button.pin);

    }

    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    printk("Set up button at %s pin %d\n", button.port->name, button.pin);

    if (led.port && !gpio_is_ready_dt(&led)) {
        printk("Error %d: LED device %s is not ready; ignoring it\n",
               ret, led.port->name);
        led.port = NULL;
    }
    if (led.port) {
        ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
        if (ret != 0) {
            printk("Error %d: failed to configure LED device %s pin %d\n",
                   ret, led.port->name, led.pin);
            led.port = NULL;
        } else {
            printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
        }
    }

    printk("Press the button\n");

    if (led.port) {
        while (1) {
            int val = gpio_pin_get_dt(&button);

            if (val >= 1) {
                if(k_mutex_lock(&mutex_evento, K_FOREVER) == 0){

                    gpio_pin_set_dt(&led, val);
                }
            }
            
            k_msleep(500);
        }
    }
}
    

void diferente(void *p1, void *p2, void *p3){
    
    int ret2;
	ret2 = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);

    while(1){
        if(k_mutex_lock(&mutex_pisca, K_FOREVER) == 0){
            if(k_mutex_lock(&mutex_evento, K_FOREVER) == 0){
                gpio_pin_toggle_dt(&led2);
                //k_msleep(5000);
                k_mutex_unlock(&mutex_pisca);
                k_mutex_unlock(&mutex_evento);
            }
        }
	    k_msleep(1000);
    }

}

//K_THREAD_DEFINE(thread_pisca, 1024, pisca, NULL, NULL, NULL, 0, 0, 0);

K_TIMER_DEFINE(timer_pisca, pisca, NULL);
K_THREAD_DEFINE(thread_existencia, 1024, existe, NULL, NULL, NULL, 0, 0, 0);

//K_THREAD_DEFINE(thread_evento, 1024, evento, NULL, NULL, NULL, 0, 0, 0);

//K_THREAD_DEFINE(thread_diferente, 1024, diferente, NULL, NULL, NULL, 0, 0, 0);
