/*
    ChibiOS/RT - Copyright (C) 2006-2007 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file serial.h
 * @brief Serial Drivers macros and structures.
 * @addtogroup Serial
 * @{
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

#if CH_USE_SERIAL_FULLDUPLEX

/** No pending conditions.*/
#define SD_NO_ERROR             0
/** Connection happened.*/
#define SD_CONNECTED            1
/** Disconnection happened.*/
#define SD_DISCONNECTED         2
/** Parity error happened.*/
#define SD_PARITY_ERROR         4
/** Framing error happened.*/
#define SD_FRAMING_ERROR        8
/** Overflow happened.*/
#define SD_OVERRUN_ERROR        16
/** Break detected.*/
#define SD_BREAK_DETECTED       32

/** Serial Driver condition flags type.*/
typedef uint8_t dflags_t;

/**
 * @brief @p FullDuplexDriver specific methods.
 */
struct _full_duplex_driver_methods {
};

/**
 * @brief @p FullDuplexDriver specific data.
 */
struct _full_duplex_driver_data {
  /**
   * Input queue, incoming data can be read from this input queue by
   * using the queues APIs.
   */
  InputQueue            iqueue;
  /**
   * Output queue, outgoing data can be written to this output queue by
   * using the queues APIs.
   */
  OutputQueue           oqueue;
  /**
   * Status Change @p EventSource. This event is generated when one or more
   * condition flags change.
   */
  EventSource           sevent;
  /**
   * I/O driver status flags.
   */
  dflags_t              flags;
};

/**
 * @brief @p FullDuplexDriver virtual methods table.
 */
struct FullDuplexDriverVMT {
  /**
   * @p BaseChannel class inherited methods.
   */
  struct _base_channel_methods m0;
  /**
   * @p BaseAsynchronousChannel class inherited methods.
   */
  struct _base_asynchronous_channel_methods m1;
  /**
   * @p FullDuplexDriver specific methods.
   */
  struct _full_duplex_driver_methods m2;
};

/**
 * @extends BaseAsynchronousChannel
 *
 * @brief Full duplex serial driver class.
 * @details This class extends @p GenericSerialDriver by adding physical I/O
 *          queues.
 */
typedef struct {
  /**
   * Virtual Methods Table.
   */
  const struct FullDuplexDriverVMT *vmt;
  /**
   * @p BaseChannel class inherited data.
   */
  struct _base_channel_data d0;
  /**
   * @p BaseAsynchronousChannel class inherited data.
   */
  struct _base_asynchronous_channel_data d1;
  /**
   * @p FullDuplexDriver specific data.
   */
  struct _full_duplex_driver_data d2;
} FullDuplexDriver;

#ifdef __cplusplus
extern "C" {
#endif
  void chFDDInit(FullDuplexDriver *sd,
                 uint8_t *ib, size_t isize, qnotify_t inotify,
                 uint8_t *ob, size_t osize, qnotify_t onotify);
  void chFDDIncomingDataI(FullDuplexDriver *sd, uint8_t b);
  msg_t chFDDRequestDataI(FullDuplexDriver *sd);
  void chFDDAddFlagsI(FullDuplexDriver *sd, dflags_t mask);
  dflags_t chFDDGetAndClearFlags(FullDuplexDriver *sd);
#ifdef __cplusplus
}
#endif

/**
 * @brief Direct output check on a @p FullDuplexDriver.
 * @details This function bypasses the indirect access to the channel and
 *          checks directly the output queue. This is faster but cannot
 *          be used to check different channels implementations.
 * @see chIOPutWouldBlock()
 */
#define chFDDPutWouldBlock(sd) chOQIsFull(&(sd)->d2.oqueue)

/**
 * @brief Direct input check on a @p FullDuplexDriver.
 * @details This function bypasses the indirect access to the channel and
 *          checks directly the input queue. This is faster but cannot
 *          be used to check different channels implementations.
 * @see chIOGetWouldBlock()
 */
#define chFDDGetWouldBlock(sd) chIQIsEmpty(&(sd)->d2.iqueue)

/**
 * @brief Direct blocking write to a @p FullDuplexDriver.
 * @details This function bypasses the indirect access to the channel and
 *          writes directly on the output queue. This is faster but cannot
 *          be used to write to different channels implementations.
 * @see chIOPut()
 */
#define chFDDPut(sd, b) chOQPut(&(sd)->d2.oqueue, b)

/**
 * @brief Direct blocking write on a @p FullDuplexDriver with timeout
 *        specification.
 * @details This function bypasses the indirect access to the channel and
 *          writes directly on the output queue. This is faster but cannot
 *          be used to write to different channels implementations.
 * @see chIOPutTimeout()
 */
#define chFDDPutTimeout(sd, b, t) chOQPutTimeout(&(sd)->d2.iqueue, b, t)

/**
 * @brief Direct blocking read from a @p FullDuplexDriver.
 * @details This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 * @see chIOGet()
 */
#define chFDDGet(sd) chIQGet(&(sd)->d2.iqueue)

/**
 * @brief Direct blocking read from a @p FullDuplexDriver with timeout
 *        specification.
 * @details This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 * @see chIOGetTimeout()
 */
#define chFDDGetTimeout(sd, t) chIQGetTimeout(&(sd)->d2.iqueue, t)

/**
 * @brief Direct non-blocking write to a @p FullDuplexDriver.
 * @details This function bypasses the indirect access to the channel and
 *          writes directly to the output queue. This is faster but cannot
 *          be used to write from different channels implementations.
 * @see chIOWrite()
 */
#define chFDDWrite(sd, b, n) chOQWrite(&(sd)->d2.oqueue, b, n)

/**
 * @brief Direct non-blocking read on a @p FullDuplexDriver.
 * @details This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 * @see chIORead()
 */
#define chFDDRead(sd, b, n) chIQRead(&(sd)->d2.iqueue, b, n)

#endif /* CH_USE_SERIAL_FULLDUPLEX */

#endif /* _SERIAL_H_ */

/** @} */