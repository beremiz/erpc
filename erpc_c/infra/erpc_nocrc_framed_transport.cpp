/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_nocrc_framed_transport.hpp"

#include "erpc_config_internal.h"
#include ENDIANNESS_HEADER
#include "erpc_message_buffer.hpp"

#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

NoCRCFramedTransport::NoCRCFramedTransport(void) :
FramedTransport()
{
}

NoCRCFramedTransport::~NoCRCFramedTransport(void) {}

uint8_t NoCRCFramedTransport::reserveHeaderSize(void)
{
    return sizeof(NoCRCFramedTransport::Header::m_messageSize);
}

erpc_status_t NoCRCFramedTransport::receive(MessageBuffer *message)
{
    Header h = { 0 };
    erpc_status_t retVal;
    uint8_t offset = 0;

    // e.g. rpmsg tty may have nullptr and buffer is assigned in receive function.
    if ((message->get() != nullptr) && (message->getLength() < reserveHeaderSize()))
    {
        retVal = kErpcStatus_MemoryError;
    }
    else
    {
#if !ERPC_THREADS_IS(NONE)
        Mutex::Guard lock(m_receiveLock);
#endif

        // Receive header first.
        retVal = underlyingReceive(message, reserveHeaderSize(), 0);
        if ((retVal == kErpcStatus_Success) && (message->getLength() < reserveHeaderSize()))
        {
            retVal = kErpcStatus_MemoryError;
        }

        if (retVal == kErpcStatus_Success)
        {
            static_cast<void>(memcpy(&h.m_messageSize, message->get(), sizeof(h.m_messageSize)));
            offset = sizeof(h.m_messageSize);

            ERPC_READ_AGNOSTIC_16(h.m_messageSize);
        }

        if (retVal == kErpcStatus_Success)
        {
            // received size can't be larger then buffer length.
            if ((h.m_messageSize + reserveHeaderSize()) > message->getLength())
            {
                retVal = kErpcStatus_ReceiveFailed;
            }
        }

        if (retVal == kErpcStatus_Success)
        {
            // rpmsg tty can receive all data in one buffer, others need second call.
            if (message->getUsed() < (h.m_messageSize + reserveHeaderSize()))
            {
                // Receive rest of the message now we know its size.
                retVal = underlyingReceive(message, h.m_messageSize, offset);
            }
        }
    }

    return retVal;
}

erpc_status_t NoCRCFramedTransport::send(MessageBuffer *message)
{
    erpc_status_t ret;
    uint16_t messageLength;
    Header h;

    messageLength = message->getUsed() - reserveHeaderSize();

    // Send header first.
    h.m_messageSize = messageLength;

    ERPC_WRITE_AGNOSTIC_16(h.m_messageSize);

    static_cast<void>(
        memcpy(message->get(), reinterpret_cast<const uint8_t *>(&h.m_messageSize), sizeof(h.m_messageSize)));

    ret = underlyingSend(message, message->getUsed(), 0);

    return ret;
}
