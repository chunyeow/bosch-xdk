/*
 * Amazon FreeRTOS
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */


/**
 * @file aws_mqtt_agent_config.h
 * @brief MQTT agent default config options.
 *
 * Ensures that the config options for MQTT agent are set to sensible
 * default values if the user does not provide one.
 */

#ifndef _AWS_MQTT_AGENT_CONFIG_H_
#define _AWS_MQTT_AGENT_CONFIG_H_

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief Controls whether or not to report usage metrics to the
 * AWS IoT broker.
 *
 * If mqttconfigENABLE_METRICS is set to 1, a string containing
 * metric information will be included in the "username" field of
 * the MQTT connect messages.
 */
#ifndef mqttconfigENABLE_METRICS
    #define mqttconfigENABLE_METRICS    ( 1 )
#endif

/**
 * @defgroup Metrics The metrics reported to the AWS IoT broker.
 *
 * If mqttconfigENABLE_METRICS is set to 1, these will be included
 * in the "username" field of MQTT connect messages.
 */
/** @{ */

#define mqttconfigMETRIC_SDK    "SDK=BoschXDK110"                    /**< The SDK used by this device. */



/** @} */

#endif /* _AWS_MQTT_AGENT_CONFIG_H_ */
