/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__NOCRC_FRAMED_TRANSPORT_H_
#define _EMBEDDED_RPC__NOCRC_FRAMED_TRANSPORT_H_

#include "erpc_config_internal.h"
#include "erpc_message_buffer.hpp"
#include "erpc_transport.hpp"
#include "erpc_framed_transport.hpp"

#include <cstring>

#if !ERPC_THREADS_IS(NONE)
#include "erpc_threading.h"
#endif

/*!
 * @addtogroup infra_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {

/*!
 * @brief Base class for framed transport layers.
 *
 * This class adds simple framing to the data transmitted and received on the
 * communications channel. This allows the transport to perform reads and writes
 * of a size known in advance. Subclasses must implement the underlyingSend() and
 * underlyingReceive() methods to actually transmit and receive data.
 *
 * Frames have a maximum size of 64kB, as a 16-bit frame size is used.
 *
 * @note This implementation currently assumes both sides of the communications channel
 *  are the same endianness.
 *
 * The frame header includes no CRC-16 for integrity checking.
 *
 * @ingroup infra_transport
 */
class NoCRCFramedTransport : public FramedTransport
{
public:
    /*! @brief Contents of the header that prefixes each message. */
    struct Header
    {
        uint16_t m_messageSize; //!< Size in bytes of the message, excluding the header.
    };

    /*!
     * @brief Constructor.
     */
    NoCRCFramedTransport(void);

    /*!
     * @brief NoCRCFramedTransport destructor
     */
    virtual ~NoCRCFramedTransport(void);

    /**
     * @brief Size of data placed in MessageBuffer before serializing eRPC data.
     *
     * @return uint8_t Amount of bytes, reserved before serialized data.
     */
    virtual uint8_t reserveHeaderSize(void) override;

    /*!
     * @brief Receives an entire message.
     *
     * The frame header and message data are received.
     *
     * The @a message is only filled with the message data, not the frame header.
     *
     * This function is blocking.
     *
     * @param[in] message Message buffer, to which will be stored incoming message.
     *
     * @retval kErpcStatus_Success When receiving was successful.
     * @retval kErpcStatus_CrcCheckFailed When receiving failed.
     * @retval other Subclass may return other errors from the underlyingReceive() method.
     */
    virtual erpc_status_t receive(MessageBuffer *message) override;

    /*!
     * @brief Function to send prepared message.
     *
     * @param[in] message Pass message buffer to send.
     *
     * @retval kErpcStatus_Success When sending was successful.
     * @retval other Subclass may return other errors from the underlyingSend() method.
     */
    virtual erpc_status_t send(MessageBuffer *message) override;

};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__NOCRC_FRAMED_TRANSPORT_H_
