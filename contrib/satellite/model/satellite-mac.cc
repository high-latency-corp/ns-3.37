/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/packet.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/uinteger.h>
#include <ns3/boolean.h>
#include <ns3/nstime.h>
#include <ns3/pointer.h>

#include "satellite-mac-tag.h"
#include "satellite-address-tag.h"
#include "satellite-time-tag.h"
#include "satellite-typedefs.h"
#include "satellite-mac.h"


NS_LOG_COMPONENT_DEFINE ("SatMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatMac);

TypeId
SatMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMac")
    .SetParent<Object> ()
    .AddConstructor<SatMac> ()
    .AddAttribute ("EnableStatisticsTags",
                   "If true, some tags will be added to each transmitted packet to assist with statistics computation",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatMac::m_isStatisticsTagsEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("NcrVersion2",
                   "NCR version used (false for 1, true for 2)",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatMac::m_ncrV2),
                   MakeBooleanChecker ())
    .AddTraceSource ("PacketTrace",
                     "Packet event trace",
                     MakeTraceSourceAccessor (&SatMac::m_packetTrace),
                     "ns3::SatTypedefs::PacketTraceCallback")
    .AddTraceSource ("Rx",
                     "A packet received",
                     MakeTraceSourceAccessor (&SatMac::m_rxTrace),
                     "ns3::SatTypedefs::PacketSenderAddressCallback")
    .AddTraceSource ("RxDelay",
                     "A packet is received with delay information",
                     MakeTraceSourceAccessor (&SatMac::m_rxDelayTrace),
                     "ns3::SatTypedefs::PacketDelayAddressCallback")
    .AddTraceSource ("RxLinkDelay",
                     "A packet is received with link delay information",
                     MakeTraceSourceAccessor (&SatMac::m_rxLinkDelayTrace),
                     "ns3::SatTypedefs::PacketDelayAddressCallback")
    .AddTraceSource ("RxJitter",
                     "A packet is received with jitter information",
                     MakeTraceSourceAccessor (&SatMac::m_rxJitterTrace),
                     "ns3::SatTypedefs::PacketJitterAddressCallback")
    .AddTraceSource ("RxLinkJitter",
                     "A packet is received with link jitter information",
                     MakeTraceSourceAccessor (&SatMac::m_rxLinkJitterTrace),
                     "ns3::SatTypedefs::PacketJitterAddressCallback")
    .AddTraceSource ("BeamServiceTime",
                     "A beam was disabled. Transmits length of last beam service time.",
                     MakeTraceSourceAccessor (&SatMac::m_beamServiceTrace),
                     "ns3::SatTypedefs::ServiceTimeCallback")
  ;
  return tid;
}

SatMac::SatMac ()
  : m_isStatisticsTagsEnabled (false),
  m_ncrV2 (false),
  m_routingUpdateCallback (),
  m_nodeInfo (),
  m_txEnabled (true),
  m_beamEnabledTime (Seconds (0)),
  m_lastDelay (0),
  m_forwardLinkRegenerationMode (SatEnums::TRANSPARENT),
  m_returnLinkRegenerationMode (SatEnums::TRANSPARENT),
  m_isRegenerative (false),
  m_satelliteAddress (),
  m_lastLinkDelay (0)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false); // this version of the constructor should not been used
}

SatMac::SatMac (uint32_t satId,
                uint32_t beamId,
                SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                SatEnums::RegenerationMode_t returnLinkRegenerationMode)
  : m_isStatisticsTagsEnabled (false),
  m_ncrV2 (false),
  m_routingUpdateCallback (),
  m_nodeInfo (),
  m_satId (satId),
  m_beamId (beamId),
  m_txEnabled (true),
  m_beamEnabledTime (Seconds (0)),
  m_lastDelay (0),
  m_forwardLinkRegenerationMode (forwardLinkRegenerationMode),
  m_returnLinkRegenerationMode (returnLinkRegenerationMode),
  m_isRegenerative (false),
  m_satelliteAddress (),
  m_lastLinkDelay (0)
{
  NS_LOG_FUNCTION (this);
}

SatMac::~SatMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  if (m_txEnabled) m_beamServiceTrace (Simulator::Now () - m_beamEnabledTime);

  m_txCallback.Nullify ();
  m_rxCallback.Nullify ();
  m_readCtrlCallback.Nullify ();
  m_reserveCtrlCallback.Nullify ();
  m_sendCtrlCallback.Nullify ();
  m_routingUpdateCallback.Nullify ();

  Object::DoDispose ();
}


void
SatMac::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo);
  NS_LOG_INFO ("Node (id)= " << nodeInfo->GetNodeId () <<
               " Node (type)= " << nodeInfo->GetNodeType () <<
               " Address= " << nodeInfo->GetMacAddress ());
  m_nodeInfo = nodeInfo;
}

uint32_t
SatMac::ReserveIdAndStoreCtrlMsgToContainer (Ptr<SatControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);

  uint32_t id = 0;

  if ( m_reserveCtrlCallback.IsNull () == false )
    {
      id = m_reserveCtrlCallback (msg);
    }
  else
    {
      NS_FATAL_ERROR ("Reserve control message (m_reserveCtrlCallback) callback is NULL!");
    }
  return id;
}

uint32_t
SatMac::SendCtrlMsgFromContainer (uint32_t sendId)
{
  NS_LOG_FUNCTION (this << sendId);

  uint32_t recvId (0);
  if ( m_sendCtrlCallback.IsNull () == false )
    {
      recvId = m_sendCtrlCallback (sendId);
    }
  else
    {
      NS_FATAL_ERROR ("Write control message (m_writeCtrlCallback) callback is NULL!");
    }
  return recvId;
}

void
SatMac::ReceiveQueueEvent (SatQueue::QueueEvent_t /*event*/, uint8_t /*flowIndex*/)
{
  NS_LOG_FUNCTION (this);
}

void
SatMac::Enable ()
{
  NS_LOG_FUNCTION (this);
  if (!m_txEnabled) m_beamEnabledTime = Simulator::Now ();
  m_txEnabled = true;
}

void
SatMac::Disable ()
{
  NS_LOG_FUNCTION (this);
  if (m_txEnabled) m_beamServiceTrace (Simulator::Now () - m_beamEnabledTime);
  m_txEnabled = false;
}

void
SatMac::SetSatelliteAddress (Address satelliteAddress)
{
  m_satelliteAddress = satelliteAddress;
  m_isRegenerative = true;
}

void
SatMac::SetTimeTag (SatPhy::PacketContainer_t packets)
{
  if (m_isStatisticsTagsEnabled)
    {
      for (SatPhy::PacketContainer_t::const_iterator it = packets.begin (); it != packets.end (); ++it)
        {
          SatMacTimeTag timeTag;
          if (!(*it)->PeekPacketTag (timeTag))
            {
              (*it)->AddPacketTag (SatMacTimeTag (Simulator::Now ()));
            }

          SatMacLinkTimeTag linkTimeTag;
          if (!(*it)->PeekPacketTag (linkTimeTag))
            {
               (*it)->AddPacketTag (SatMacLinkTimeTag (Simulator::Now ()));
            }
        }
    }
}

void
SatMac::SendPacket (SatPhy::PacketContainer_t packets, uint32_t carrierId, Time duration, SatSignalParameters::txInfo_s txInfo)
{
  NS_LOG_FUNCTION (this);

  // Add a SatMacTimeTag tag for packet delay computation at the receiver end.
  SetTimeTag (packets);

  // Update local destination MAC tag with satellite one if satellite is regenerative
  if (m_isRegenerative)
    {
      for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
           it != packets.end (); ++it)
        {
          SatMacTag mTag;
          bool success = (*it)->RemovePacketTag (mTag);

          // MAC tag found
          if (success)
            {
              mTag.SetDestAddress (Mac48Address::ConvertFrom (m_satelliteAddress));
              (*it)->AddPacketTag (mTag);
            }
        }
    }

  /**
   * The transmitted packets are gone through to check whether the PHY transmission
   * contains control packets. If a control packet is found, its control tag is peeked
   * and the control message is added to the control message container with control
   * message id. Only made if not on a satellite.
   */
  for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
       it != packets.end (); ++it)
    {
      SatControlMsgTag cTag;
      bool success = (*it)->RemovePacketTag (cTag);

      // Control tag found
      if (success)
        {
          uint32_t sendId = cTag.GetMsgId ();

          // Add the msg to container and receive the used recv id.
          uint32_t recvId = SendCtrlMsgFromContainer (sendId);

          // Store the recv id to tag and add the tag back to the packet
          cTag.SetMsgId (recvId);
          (*it)->AddPacketTag (cTag);

          if (cTag.GetMsgType () == SatControlMsgTag::SAT_NCR_CTRL_MSG)
            {
              Ptr<SatNcrMessage> ncrMsg = m_ncrMessagesToSend.front ();
              m_ncrMessagesToSend.pop ();
              uint8_t lastSOFSize = m_ncrV2 ? 3 : 1;
              ncrMsg->SetNcrDate (m_lastSOF.size () == lastSOFSize ? m_lastSOF.front ().GetNanoSeconds ()*0.027 : 0);
            }
        }
    }

  // Use call back to send packet to lower layer
  m_txCallback (packets, carrierId, duration, txInfo);
}

void
SatMac::RxTraces (SatPhy::PacketContainer_t packets)
{
  NS_LOG_FUNCTION (this);

  if (m_isStatisticsTagsEnabled)
    {
      for (SatPhy::PacketContainer_t::const_iterator it1 = packets.begin ();
           it1 != packets.end (); ++it1)
        {
          // Remove packet tag
          SatMacTag macTag;
          bool mSuccess = (*it1)->PeekPacketTag (macTag);
          if (!mSuccess)
            {
              NS_FATAL_ERROR ("MAC tag was not found from the packet!");
            }

          // If the packet is intended for this receiver
          Mac48Address destAddress = macTag.GetDestAddress ();

          if (destAddress == m_nodeInfo->GetMacAddress ())
            {
              Address addr; // invalid address.

              bool isTaggedWithAddress = false;
              ByteTagIterator it2 = (*it1)->GetByteTagIterator ();

              while (!isTaggedWithAddress && it2.HasNext ())
                {
                  ByteTagIterator::Item item = it2.Next ();

                  if (item.GetTypeId () == SatAddressTag::GetTypeId ())
                    {
                      NS_LOG_DEBUG (this << " contains a SatAddressTag tag:"
                                         << " start=" << item.GetStart ()
                                         << " end=" << item.GetEnd ());
                      SatAddressTag addrTag;
                      item.GetTag (addrTag);
                      addr = addrTag.GetSourceAddress ();
                      isTaggedWithAddress = true; // this will exit the while loop.
                    }
                }

              m_rxTrace (*it1, addr);

              SatMacTimeTag timeTag;
              if ((*it1)->RemovePacketTag (timeTag))
                {
                  NS_LOG_DEBUG (this << " contains a SatMacTimeTag tag");
                  Time delay = Simulator::Now () - timeTag.GetSenderTimestamp ();
                  m_rxDelayTrace (delay, addr);
                  if (m_lastDelay.IsZero() == false)
                    {
                      Time jitter = Abs (delay - m_lastDelay);
                      m_rxJitterTrace (jitter, addr);
                    }
                  m_lastDelay = delay;
                }
              SatMacLinkTimeTag linkTimeTag;
              if ((*it1)->RemovePacketTag (linkTimeTag))
                {
                  NS_LOG_DEBUG (this << " contains a SatMacLinkTimeTag tag");
                  Time delay = Simulator::Now () - linkTimeTag.GetSenderLinkTimestamp ();
                  m_rxLinkDelayTrace (delay, addr);
                  if (m_lastLinkDelay.IsZero() == false)
                    {
                      Time jitter = Abs (delay - m_lastLinkDelay);
                      m_rxLinkJitterTrace (jitter, addr);
                    }
                  m_lastLinkDelay = delay;
                }
            } // end of `if (destAddress == m_nodeInfo->GetMacAddress () || destAddress.IsBroadcast ())`
        } // end of `for it1 = packets.begin () -> packets.end ()`
    } // end of `if (m_isStatisticsTagsEnabled)`
}

void
SatMac::SetTransmitCallback (SatMac::TransmitCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_txCallback = cb;
}

void
SatMac::SetReceiveCallback (SatMac::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}

void
SatMac::SetLoraReceiveCallback (SatMac::LoraReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxLoraCallback = cb;
}

void
SatMac::SetReadCtrlCallback (SatMac::ReadCtrlMsgCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_readCtrlCallback = cb;
}

void
SatMac::SetReserveCtrlCallback (SatMac::ReserveCtrlMsgCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_reserveCtrlCallback = cb;
}

void
SatMac::SetSendCtrlCallback (SatMac::SendCtrlMsgCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_sendCtrlCallback = cb;
}

void
SatMac::SetRoutingUpdateCallback (SatMac::RoutingUpdateCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_routingUpdateCallback = cb;
}


} // namespace ns3
