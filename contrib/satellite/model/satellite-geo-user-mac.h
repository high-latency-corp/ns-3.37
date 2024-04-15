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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_GEO_USER_MAC_H
#define SATELLITE_GEO_USER_MAC_H

#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/address.h>

#include "satellite-phy.h"
#include "satellite-mac.h"
#include "satellite-geo-mac.h"
#include "satellite-geo-user-llc.h"
#include "satellite-signal-parameters.h"
#include "satellite-fwd-link-scheduler.h"


namespace ns3 {

/**
 * \ingroup satellite
 *
 * The SatGeoUserMac models the user link MAC layer of the
 * satellite node.
 */
class SatGeoUserMac : public SatGeoMac
{
public:
  /**
   * Default constructor
   */
  SatGeoUserMac (void);

  /**
   * Construct a SatGeoUserMac
   *
   * This is the constructor for the SatGeoUserMac
   *
   * \param satId ID of sat for UT
   * \param beamId ID of beam for UT
   * \param forwardLinkRegenerationMode Forward link regeneration mode
   * \param returnLinkRegenerationMode Return link regeneration mode
   */
  SatGeoUserMac (uint32_t satId, uint32_t beamId,
                 SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                 SatEnums::RegenerationMode_t returnLinkRegenerationMode);

  /**
   * Destructor for SatGeoUserMac
   */
  virtual ~SatGeoUserMac ();


  /**
   * inherited from Object
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  virtual void DoInitialize (void);

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose (void);

  /**
   * \brief Add new packet to the LLC queue.
   * \param packet Packets to be sent.
   */
  virtual void EnquePacket (Ptr<Packet> packet);

  /**
   * Receive packet from lower layer.
   *
   * \param packets Pointers to packets received.
   * \param rxParams The parameters associated to these packets.
   */
  void Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> txParams);

  void ReceiveSignalingPacket (Ptr<Packet> packet);

protected:
  /**
   * \brief Get the link TX direction. Must be implemented by child clases.
   * \return The link TX direction
   */
  virtual SatEnums::SatLinkDir_t GetSatLinkTxDir ();

  /**
   * \brief Get the link RX direction. Must be implemented by child clases.
   * \return The link RX direction
   */
  virtual SatEnums::SatLinkDir_t GetSatLinkRxDir ();

  /**
   * \brief Get the UT address associated to this RX packet.
   *        In this class, this is the source address
   * \param packet The packet to consider
   * \return The address of associated UT
   */
  virtual Address GetRxUtAddress (Ptr<Packet> packet);

private:

};

}

#endif /* SATELLITE_GEO_USER_MAC_H */
