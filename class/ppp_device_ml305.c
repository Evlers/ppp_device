/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2021-06-23   HelloByeAll    the first version
 */

#include <ppp_device.h>
#include <ppp_chat.h>
#include <rtdevice.h>

#define DBG_TAG    "ppp.ml305"

#ifdef PPP_DEVICE_DEBUG
#define DBG_LVL   DBG_LOG
#else
#define DBG_LVL   DBG_INFO
#endif
#include <rtdbg.h>

#define ML305POWER_ON  PIN_HIGH
#define ML305POWER_OFF PIN_LOW
#ifndef ML305POWER_PIN
#define ML305POWER_PIN -1
#endif
#define ML305WARTING_TIME_BASE 2000

#define PPP_DAIL_CMD         "ATD*99***1#"                          /* redefine DAIL CMD */
#ifndef PKG_USING_CMUX
static const struct modem_chat_data rst_mcd[] =
{
    {"AT+MREBOOT",   MODEM_CHAT_RESP_NOT_NEED,        30, 1, RT_FALSE},
    {"AT",           MODEM_CHAT_RESP_OK,              30, 10,RT_FALSE},
    {"+++",          MODEM_CHAT_RESP_NOT_NEED,        30, 1, RT_TRUE},
    {"ATH",          MODEM_CHAT_RESP_OK,              30, 1, RT_FALSE},
};

static const struct modem_chat_data mcd[] =
{
    {"AT",           MODEM_CHAT_RESP_OK,              10, 1, RT_FALSE},
    {"ATE0",         MODEM_CHAT_RESP_OK,              1,  1, RT_FALSE},
    {"AT+CPIN?",     MODEM_CHAT_RESP_READY,           10, 1, RT_FALSE},
    {"AT+CFUN?",     MODEM_CHAT_RESP_OK,              10, 1, RT_FALSE},
    {"AT+CSQ",       MODEM_CHAT_RESP_OK,              10, 1, RT_FALSE},
    {PPP_APN_CMD,    MODEM_CHAT_RESP_OK,              1,  5, RT_FALSE},
    {"AT+CGACT=1,1", MODEM_CHAT_RESP_OK,              10, 1, RT_FALSE},
    {PPP_DAIL_CMD,   MODEM_CHAT_RESP_CONNECT,         1, 30, RT_FALSE},
};
#else
static const struct modem_chat_data mcd[] =
{
    {"AT",           MODEM_CHAT_RESP_NOT_NEED,        10, 1, RT_FALSE},
    {PPP_APN_CMD,    MODEM_CHAT_RESP_NOT_NEED,        1,  5, RT_FALSE},
    {PPP_DAIL_CMD,   MODEM_CHAT_RESP_NOT_NEED,        1, 30, RT_FALSE},
};
#endif

static rt_err_t ppp_ml305prepare(struct ppp_device *device)
{
#ifndef PKG_USING_CMUX
    if (device->power_pin >= 0)
    {
        rt_pin_write(device->power_pin, ML305POWER_OFF);
        rt_thread_mdelay(ML305WARTING_TIME_BASE / 20);
        rt_pin_write(device->power_pin, ML305POWER_ON);
        rt_thread_mdelay(ML305WARTING_TIME_BASE);
    }
    else
    {
        rt_err_t err;
        err = modem_chat(device->uart, rst_mcd, sizeof(rst_mcd) / sizeof(rst_mcd[0]));
        if (err)
            return err;
    }
#endif
    return modem_chat(device->uart, mcd, sizeof(mcd) / sizeof(mcd[0]));
}

/* ppp_mlm305ops for ppp_device_ops , a common interface */
static struct ppp_device_ops ml305ops =
{
    .prepare = ppp_ml305prepare,
};

/**
 * register air720 into ppp_device
 *
 * @return  =0:   ppp_device register successfully
 *          <0:   ppp_device register failed
 */
int ppp_ml305register(void)
{
    struct ppp_device *ppp_device = RT_NULL;

    ppp_device = rt_malloc(sizeof(struct ppp_device));
    if(ppp_device == RT_NULL)
    {
        LOG_E("No memory for ml305 ppp_device.");
        return -RT_ENOMEM;
    }

    ppp_device->power_pin = ML305POWER_PIN;
    if (ppp_device->power_pin >= 0)
    {
        rt_pin_mode(ppp_device->power_pin, PIN_MODE_OUTPUT);
        rt_pin_write(ppp_device->power_pin, ML305POWER_ON);
        rt_thread_mdelay(ML305WARTING_TIME_BASE);
    }
    ppp_device->ops = &ml305ops;

    LOG_D("ppp ml305 is registering ppp_device");

    return ppp_device_register(ppp_device, PPP_DEVICE_NAME, RT_NULL, RT_NULL);
}
INIT_COMPONENT_EXPORT(ppp_ml305register);
