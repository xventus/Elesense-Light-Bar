//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   application.h
/// @author Petr Vanek

#include "application.h"
#include "hardware.h"
#include "content_file.h"
#include "driver/uart.h"
#include "button.h"
#include "key_val.h"

// global application instance as singleton and instance acquisition.


Application *Application::getInstance()
{
    static Application instance;
    return &instance;
}

Application::Application() :    _heartBeat(HEART_BEAT_LED), 
                                _btnTask(A_BUTTON, B_BUTTON, FLASH_BUTTON)
{
}

Application::~Application()
{
    done();
}

void Application::checkRenewAP() 
{
     // reset AP check
    Button b1(A_BUTTON);
    Button b2(B_BUTTON);
    KeyVal& kv = KeyVal::getInstance();
   
    if (b1.isPressed() && b2.isPressed()) {
		kv.writeString(literals::kv_ssid, "");
    }
}

void Application::init()
{
    // Begin initialization of core functions so that individual 
    // wrappers no longer need to execute
    
    // initi file system
    ConentFile::initFS();

    // initialize NVS
    KeyVal& kv = KeyVal::getInstance();
    kv.init(literals::kv_namespace ,true, false);

    // initialize newtwork interfaces
    ESP_ERROR_CHECK(esp_netif_init());

    // default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // serial line & LCS12
    gpio_set_direction(LSC_SET, GPIO_MODE_OUTPUT);
	gpio_set_direction(LSC_CS, GPIO_MODE_OUTPUT);
	gpio_set_level(LSC_CS, 0);
	gpio_set_level(LSC_SET, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS); 
 
	 const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_APB
    };

   
    uart_driver_install(LCS_UART, 2048, 0, 0, NULL, 0); 
    uart_param_config(LCS_UART, &uart_config);
    uart_set_pin(LCS_UART, LSC_TX_PIN, LSC_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

   // checkRenewAP();

}

void Application::run()
{
    // task create
    do
    {
        if (!_heartBeat.init(literals::tsk_led, tskIDLE_PRIORITY + 1ul, configMINIMAL_STACK_SIZE*10))
            break;

        if (!_wifiTask.init(literals::tsk_web, tskIDLE_PRIORITY + 1ul, 4086))
            break;

        if (!_webTask.init(literals::tsk_wifi, tskIDLE_PRIORITY + 1ul, 4086))
            break;

        if (!_lcs12cTask.init(literals::tsk_lcs, tskIDLE_PRIORITY + 1ul, 4086))
           break;

        if (!_btnTask.init(literals::btn_lcs, tskIDLE_PRIORITY + 1ul, 4086)) 
           break;

    } while (false);
    
    
}

void Application::done()
{
    // fail 
    KeyVal& kv = KeyVal::getInstance();
    kv.done();
}

Application *Application::operator->()
{
    return Application::getInstance();
}

Application  *Application::operator->() const
{
    return Application::getInstance();
}
