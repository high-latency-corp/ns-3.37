/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#ifndef SATELLITE_GENERIC_STREAM_ENCAPSULATOR_ARQ
#define SATELLITE_GENERIC_STREAM_ENCAPSULATOR_ARQ


#include <map>

#include <ns3/event-id.h>
#include <ns3/mac48-address.h>

#include "satellite-generic-stream-encapsulator.h"
#include "satellite-arq-sequence-number.h"
#include "satellite-arq-buffer-context.h"
#include "satellite-control-message.h"


namespace ns3 {


/**
 * \ingroup satellite
 *
 * \brief SatGenericStreamEncapsulatorArq class is inherited from the
 * SatGenericStreamEncapsulator class, which is used in FWD link for
 * encapsulation and fragmentation of higher layer packets. This class
 * implements the ARQ retransmission logic in both transmission and
 * reception side for GSE.
 *
 * Implemented algorithm is based on Selective Repeat ARQ.
 * Retransmission logic is based on a set of timers and added increasing
 * sequence numbers for each sent packet. When a packet is received, and
 * ACK is sent to the receiver with a proper sequence number.
 */
class SatGenericStreamEncapsulatorArq : public SatGenericStreamEncapsulator
{
public:
  /**
   * Default constructor, not used
   */
  SatGenericStreamEncapsulatorArq ();

  /**
   * Constructor
   * \param encapAddress MAC addressd of encapsulator
   * \param decapAddress MAC addressd of decapsulator
   * \param sourceE2EAddress E2E source MAC addressd of packets (used to set SatAddressE2ETag)
   * \param destE2EAddress E2E destination MAC addressd of packets (used to set SatAddressE2ETag)
   * \param flowId Flow identifier
   * \param additionalHeaderSize Additional value in to take into account when pulling packets to represent E2E tags
   */
  SatGenericStreamEncapsulatorArq (Mac48Address encapAddress,
                                   Mac48Address decapAddress,
                                   Mac48Address sourceE2EAddress,
                                   Mac48Address destE2EAddress,
                                   uint8_t flowId,
                                   uint32_t additionalHeaderSize = 0);

  /**
   * Destructor for SatGenericStreamEncapsulatorArq
   */
  virtual ~SatGenericStreamEncapsulatorArq ();

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * \brief Notify a Tx opportunity to this encapsulator.
   * \param bytes Notified tx opportunity bytes from lower layer
   * \param &bytesLeft Bytes left after this TxOpportunity in SatQueue
   * \param &nextMinTxO Minimum TxO after this TxO
   * \return A GSE PDU
   */
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft, uint32_t &nextMinTxO);

  /**
   * \brief Receive a packet, thus decapsulate and defragment/deconcatenate
   * if needed. The decapsuled/defragmented HL PDU is forwarded back to
   * LLC and to upper layer.
   * \param p A packet pointer received from lower layer
   */
  virtual void ReceivePdu (Ptr<Packet> p);

  /**
   * \brief Receive a control message (ARQ ACK)
   * \param ack Control message pointer received from lower layer
   */
  virtual void ReceiveAck (Ptr<SatArqAckMessage> ack);

  /**
   * \brief Get the buffered packets for this encapsulator
   * \return uint32_t buffered bytes
   */
  virtual uint32_t GetTxBufferSizeInBytes () const;

private:
  /**
   * \brief ARQ Tx timer has expired. The PDU will be flushed, if the maximum
   * retransmissions has been reached. Otherwise the packet will be resent.
   * \param seqNo Sequence number
   */
  void ArqReTxTimerExpired (uint8_t seqNo);

  /**
   * \brief Clean-up a certain sequence number
   * \param sequenceNumber Sequence number
   */
  void CleanUp (uint8_t sequenceNumber);

  /**
   * \brief Convert the 8-bit sequence number value from ARQ header into
   * 32-bit continuous sequence number stream at the receiver.
   * \param seqNo 8-bit sequence number
   * \return uint32_t 32-bit sequence number
   */
  uint32_t ConvertSeqNo (uint8_t seqNo) const;

  /**
   * \brief Reassemble and receive the received PDUs if possible
   */
  void ReassembleAndReceive ();

  /**
   * \brief Rx waiting timer for a PDU has expired
   * \param sn Sequence number
   */
  void RxWaitingTimerExpired (uint32_t sn);

  /**
   * \brief Send ACK for a given sequence number
   * \param seqNo Sequence number
   */
  void SendAck (uint8_t seqNo) const;

  /**
   * Sequence number handler
   */
  Ptr<SatArqSequenceNumber> m_seqNo;

  /**
   * Transmitted and retransmission context buffer
   */
  std::map < uint8_t, Ptr<SatArqBufferContext> > m_txedBuffer;       // Transmitted packets buffer
  std::map < uint8_t, Ptr<SatArqBufferContext> > m_retxBuffer;       // Retransmission buffer
  uint32_t m_retxBufferSize;
  uint32_t m_txedBufferSize;

  /**
   * Maximum number of retransmissions
   */
  uint32_t m_maxNoOfRetransmissions;

  /**
   * Retransmission timer, i.e. when to retransmit a packet if
   * a ACK has not been received.
   */
  Time m_retransmissionTimer;

  /**
   * ARQ window size, i.e. how many sequential sequence numbers may be in
   * use simultaneously.
   */
  uint32_t m_arqWindowSize;

  /**
   * ARQ header size in Bytes
   */
  uint32_t m_arqHeaderSize;

  /**
   * Next expected sequence number at the packet reception
   */
  uint32_t m_nextExpectedSeqNo;

  /**
   * Waiting time for waiting a certain SN to be received.
   */
  Time m_rxWaitingTimer;

  /**
   * key = sequence number
   * value = GSE packet
   */
  std::map<uint32_t, Ptr<SatArqBufferContext> > m_reorderingBuffer;
};


} // namespace ns3

#endif // SATELLITE_GENERIC_STREAM_ENCAPSULATOR_ARQ
