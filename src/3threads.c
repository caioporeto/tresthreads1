#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/__assert.h>
#include <string.h>

#define MY_STACK_SIZE 1024
#define MY_PRIORITY 0

#define SW0_NODE	DT_ALIAS(sw0)
#define LED2_NODE DT_ALIAS(led2)
#define LED0_NODE DT_ALIAS(led0)

K_MUTEX_DEFINE(mutex1);
K_MUTEX_DEFINE(mutex2);

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;
static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
static struct gpio_dt_spec led_1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});
static struct gpio_dt_spec led_2 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});


int aperta_botao = 1;

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
	printk("pao\n");
	aperta_botao = !aperta_botao;
//VARIAVEL GLOBAL
}

void F1(void) {
	int ret1;
	ret1 = gpio_pin_configure_dt(&led_1, GPIO_OUTPUT_ACTIVE);
    while(1){
        if((k_mutex_lock(&mutex1, K_FOREVER)) == 0) {
			//printk("bloqueio\n");
			gpio_pin_toggle_dt(&led_1); // Atualiza o LED
            k_msleep(250);
            k_mutex_unlock(&mutex1);
			//printk("libera\n");
            k_msleep(250);
        }
        else {

        }
    }
}

void F2(void) {
 	int ret;

	if (!gpio_is_ready_dt(&button)) {
		printk("Error: button device %s is not ready\n",
		       button.port->name);
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button.port->name, button.pin);
		return 0;
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

			if (aperta_botao == 1) { // Se a leitura do botão foi bem-sucedida
				if ((k_mutex_lock(&mutex2, K_FOREVER)) == 0) { // Tenta adquirir o mutex até conseguir
					gpio_pin_set_dt(&led, aperta_botao); // Atualiza o LED
					//printk("bloqueio mutex2 %d", k_mutex_lock(&mutex2, K_FOREVER));
				}
			} else if (aperta_botao == 0) {
				k_mutex_unlock(&mutex2); // Libera o mutex
				gpio_pin_set_dt(&led, aperta_botao);
				//printk("libera mutex2\n");
			}

			k_msleep(500); // Aguarda 500ms antes da próxima iteração
		}
	}	
}

void F3(void) {
	int ret2;
	ret2 = gpio_pin_configure_dt(&led_2, GPIO_OUTPUT_ACTIVE);
	while(1) {
		if ((k_mutex_lock(&mutex1, K_FOREVER)) == 0) { // se conseguir pegar o primeiro
			printk("peguei o 1\n");
			if ((k_mutex_lock(&mutex2, K_MSEC(100))) == 0) { // se conseguir pegar o segundo
			printk ("rodei\n");
			gpio_pin_set_dt(&led_2, 1);
			k_msleep(200);
			k_mutex_unlock(&mutex2);
			k_mutex_unlock(&mutex1);
			}
			else { // se nao conseguir pegar o 2, solta o 1
				k_mutex_unlock(&mutex1); 
				//printk("larguei o 1");
			}
		}
		else {
			printk("Falha ao adquirir o mutex1\n");
		}
		gpio_pin_set_dt(&led_2, 0);
		k_msleep(1000);
	}
}

K_THREAD_DEFINE(F1_id, MY_STACK_SIZE, F1, NULL, NULL, NULL, MY_PRIORITY, 0, 0);
K_THREAD_DEFINE(F2_id, MY_STACK_SIZE, F2, NULL, NULL, NULL, MY_PRIORITY, 0, 0);
K_THREAD_DEFINE(F3_id, MY_STACK_SIZE, F3, NULL, NULL, NULL, MY_PRIORITY, 0, 0);

K_TIMER_DEFINE(F1_timer, F1, NULL);
K_TIMER_DEFINE(F2_timer, F2, NULL);
K_TIMER_DEFINE(F3_timer, F3, NULL);