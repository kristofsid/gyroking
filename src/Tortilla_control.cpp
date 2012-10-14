/*
 *  Copyright (C) 2012 Xo Wang, Aaron Fan
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL XO
 *  WANG BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 *  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  Except as contained in this notice, the name of Xo Wang shall not be
 *  used in advertising or otherwise to promote the sale, use or other dealings
 *  in this Software without prior written authorization from Xo Wang.
 */

#include "ch.h"
#include "hal.h"

#include "Tortilla.h"
#include "chprintf.h"
#include "A4960.h"
#include "ADS1259.h"

#include <algorithm>

#include <stdint.h>
#include <cmath>

Tortilla::Tortilla(A4960 &m1, A4960 &m2, ADS1259 &adc, ICUDriver *icup, SerialDriver *sdp) :
        m1(m1),
        m2(m2),
        adc(adc),
        icup(icup),
        sdp(sdp),
        gyroBias(0),
        lastRate(0),
        currentRate(0),
        theta(0),
        throttle(0),
        joyAngle(0),
        joyMag(0) {
}

void Tortilla::initGyroBias() {
    int32_t adcReading;
    int64_t sumReadings = 0;
    size_t numReadings = 0;
    systime_t ticks = chTimeNow();
    for (size_t i = 0; i < 6000U; i++) {
        if (adc.read(adcReading) && i >= 1000) {
            numReadings++;
            sumReadings += adcReading;
        }

        ticks += MS2ST(1);
        chThdSleepUntil(ticks);
    }
    gyroBias = sumReadings / numReadings;
}

static constexpr float SCALE_RAW_TO_RADSEC = float(64000.0 * (M_PI / 180.0) / double(0x7FFFFFUL));

static constexpr size_t LOOP_DELAY_US = 1000;
static constexpr systime_t LOOP_DELAY = US2ST(LOOP_DELAY_US);
static constexpr float DT = LOOP_DELAY_US / 1000000.0;

void Tortilla::fastLoop() {
    initGyroBias();
    palClearPad(GPIOB, GPIOB_LEDY);
    palClearPad(GPIOB, GPIOB_LED1);

    systime_t ticks = chTimeNow();
    while (true) {
        int32_t adcReading;
        if (adc.read(adcReading)) {
            const float rate = SCALE_RAW_TO_RADSEC * (adcReading - gyroBias);

            lastRate = currentRate;
            currentRate = rate;
            theta += (currentRate) * DT; // euler integration
//            theta += 0.5f * (currentRate + lastRate) * DT; // trapezoidal integration
            theta = ::remainderf(theta, M_TWOPI);
        }

        pwmcnt_t m1Speed = throttle;
        pwmcnt_t m2Speed = throttle;

        const bool blue = ::fabsf(theta) < float(M_PI / 10.0);
        palWritePad(GPIOC, GPIOC_LEDB, blue);
        if (joyMag == 0.f) {
            const bool yellow = !blue && ::fabsf(::remainderf(theta * 5.f, M_TWOPI)) < float(M_PI_2);;
            palWritePad(GPIOB, GPIOB_LEDY, yellow);
        } else {
            const bool yellow = fabsf(::remainderf(theta - joyAngle, M_TWOPI)) < std::max(0.2f, ::fabsf(joyMag * M_PI_4));
            if (yellow) {
                m1Speed = PWM_PERIOD;
                m2Speed = 0;
            }
            palWritePad(GPIOB, GPIOB_LEDY, yellow);
        }

        m1.setWidth(m1Speed);
        m2.setWidth(m2Speed);

        ticks += LOOP_DELAY;
        chThdSleepUntil(ticks);
    }
}
