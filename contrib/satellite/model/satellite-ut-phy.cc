/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/double.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>

#include "satellite-utils.h"
#include "satellite-ut-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-tx.h"
#include "satellite-channel.h"
#include "satellite-mac.h"
#include "satellite-signal-parameters.h"
#include "satellite-channel-estimation-error-container.h"


NS_LOG_COMPONENT_DEFINE ("SatUtPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUtPhy);

TypeId
SatUtPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatUtPhy")
    .SetParent<SatPhy> ()
    .AddConstructor<SatUtPhy> ()
    .AddAttribute ("PhyRx", "The PhyRx layer attached to this phy.",
                   PointerValue (),
                   MakePointerAccessor (&SatUtPhy::GetPhyRx, &SatUtPhy::SetPhyRx),
                   MakePointerChecker<SatPhyRx> ())
    .AddAttribute ("PhyTx", "The PhyTx layer attached to this phy.",
                   PointerValue (),
                   MakePointerAccessor (&SatUtPhy::GetPhyTx, &SatUtPhy::SetPhyTx),
                   MakePointerChecker<SatPhyTx> ())
    .AddAttribute ( "RxTemperatureDbk",
                    "RX noise temperature in UT in dBK.",
                    DoubleValue (24.6),  // ~290K
                    MakeDoubleAccessor (&SatPhy::GetRxNoiseTemperatureDbk, &SatPhy::SetRxNoiseTemperatureDbk),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("RxMaxAntennaGainDb", "Maximum RX gain in dB",
                   DoubleValue (44.60),
                   MakeDoubleAccessor (&SatPhy::GetRxAntennaGainDb, &SatPhy::SetRxAntennaGainDb),
                   MakeDoubleChecker<double_t> ())
    .AddAttribute ("TxMaxAntennaGainDb", "Maximum TX gain in dB",
                   DoubleValue (45.20),
                   MakeDoubleAccessor (&SatPhy::GetTxAntennaGainDb, &SatPhy::SetTxAntennaGainDb),
                   MakeDoubleChecker<double_t> ())
    .AddAttribute ("TxMaxPowerDbw", "Maximum TX power in dB",
                   DoubleValue (4.00),
                   MakeDoubleAccessor (&SatPhy::GetTxMaxPowerDbw, &SatPhy::SetTxMaxPowerDbw),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxOutputLossDb", "TX Output loss in dB",
                   DoubleValue (0.50),
                   MakeDoubleAccessor (&SatPhy::GetTxOutputLossDb, &SatPhy::SetTxOutputLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPointingLossDb", "TX Pointing loss in dB",
                   DoubleValue (1.00),
                   MakeDoubleAccessor (&SatPhy::GetTxPointingLossDb, &SatPhy::SetTxPointingLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxOboLossDb", "TX OBO loss in dB",
                   DoubleValue (0.50),
                   MakeDoubleAccessor (&SatPhy::GetTxOboLossDb, &SatPhy::SetTxOboLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxAntennaLossDb", "TX Antenna loss in dB",
                   DoubleValue (1.00),
                   MakeDoubleAccessor (&SatPhy::GetTxAntennaLossDb, &SatPhy::SetTxAntennaLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxAntennaLossDb", "RX Antenna loss in dB",
                   DoubleValue (0.00),
                   MakeDoubleAccessor (&SatPhy::GetRxAntennaLossDb, &SatPhy::SetRxAntennaLossDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DefaultFadingValue", "Default value for fading",
                   DoubleValue (1.00),
                   MakeDoubleAccessor (&SatPhy::GetDefaultFading, &SatPhy::SetDefaultFading),
                   MakeDoubleChecker<double_t> ())
    .AddAttribute ("OtherSysIfCOverIDb",
                   "Other system interference, C over I in dB.",
                   DoubleValue (24.7),
                   MakeDoubleAccessor (&SatUtPhy::m_otherSysInterferenceCOverIDb),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("AntennaReconfigurationDelay",
                   "Delay of antenna reconfiguration when performing handover",
                   TimeValue (Seconds (0.0)),
                   MakeTimeAccessor (&SatUtPhy::m_antennaReconfigurationDelay),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId
SatUtPhy::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

SatUtPhy::SatUtPhy (void)
  : m_otherSysInterferenceCOverIDb (24.7),
  m_otherSysInterferenceCOverI (SatUtils::DbToLinear (m_otherSysInterferenceCOverIDb)),
  m_antennaReconfigurationDelay (Seconds (0.0))
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatUtPhy default constructor is not allowed to be used");
}

SatUtPhy::SatUtPhy (SatPhy::CreateParam_t &params,
                    Ptr<SatLinkResults> linkResults,
                    SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters,
                    Ptr<SatSuperframeConf> superFrameConf,
                    SatEnums::RegenerationMode_t forwardLinkRegenerationMode)
  : SatPhy (params),
  m_antennaReconfigurationDelay (Seconds (0.0))
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());

  m_otherSysInterferenceCOverI = SatUtils::DbToLinear (m_otherSysInterferenceCOverIDb);

  parameters.m_rxTemperatureK = SatUtils::DbToLinear (SatPhy::GetRxNoiseTemperatureDbk ());
  parameters.m_aciIfWrtNoiseFactor = 0.0;
  parameters.m_extNoiseDensityWhz = 0.0;
  parameters.m_rxMode = SatPhyRxCarrierConf::NORMAL;
  parameters.m_linkRegenerationMode = forwardLinkRegenerationMode;
  parameters.m_chType = SatEnums::FORWARD_USER_CH;

  Ptr<SatPhyRxCarrierConf> carrierConf = CreateObject<SatPhyRxCarrierConf> (parameters);

  if (linkResults)
    {
      carrierConf->SetLinkResults (linkResults);
    }

  carrierConf->SetAdditionalInterferenceCb (MakeCallback (&SatUtPhy::GetAdditionalInterference, this));

  SatPhy::ConfigureRxCarriers (carrierConf, superFrameConf);
}

SatUtPhy::~SatUtPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
SatUtPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
SatUtPhy::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  Object::DoInitialize ();
}

double
SatUtPhy::GetAdditionalInterference ()
{
  NS_LOG_FUNCTION (this);

  return m_otherSysInterferenceCOverI;
}


void
SatUtPhy::PerformHandover (uint32_t beamId)
{
  NS_LOG_FUNCTION (this << beamId);

  // disconnect current SatChannels
  SatChannelPair::ChannelPair_t channels = m_retrieveChannelPair (m_satId, m_beamId);
  m_phyTx->ClearChannel ();
  channels.first->RemoveRx (m_phyRx);

  // perform "physical" beam handover
  SetBeamId (beamId);
  Simulator::Schedule (m_antennaReconfigurationDelay, &SatUtPhy::AssignNewSatChannels, this);
}


void
SatUtPhy::AssignNewSatChannels ()
{
  NS_LOG_FUNCTION (this);

  // Fetch channels for current beam
  SatChannelPair::ChannelPair_t channels = m_retrieveChannelPair (m_satId, m_beamId);
  Ptr<SatChannel> forwardLink = channels.first;
  Ptr<SatChannel> returnLink = channels.second;

  // Assign channels
  NS_LOG_INFO ("Setting new Tx on channel " << returnLink);
  m_phyTx->SetChannel (returnLink);
  forwardLink->AddRx (m_phyRx);
}

SatEnums::SatLinkDir_t
SatUtPhy::GetSatLinkTxDir ()
{
  return SatEnums::LD_RETURN;
}

SatEnums::SatLinkDir_t
SatUtPhy::GetSatLinkRxDir ()
{
  return SatEnums::LD_FORWARD;
}

bool
SatUtPhy::IsTxPossible (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phyTx->CanTransmit ();
}

void
SatUtPhy::Receive (Ptr<SatSignalParameters> rxParams, bool phyError)
{
  NS_LOG_FUNCTION (this << rxParams << phyError);

  uint8_t slice = rxParams->m_txInfo.sliceId;

  if (rxParams->m_txInfo.frameType == SatEnums::DUMMY_FRAME)
    {
      NS_LOG_INFO ("Dummy frame receive, it is not decoded");
    }
  else if ((slice == 0) || (m_slicesSubscribed.count(slice) != 0))

    {
      // Slice is zero or is in the subscription list, we decode and forward to upper layers
      SatPhy::Receive (rxParams, phyError);
    }
  else
    {
      NS_LOG_INFO ("Slice of BBFrame (" << (uint32_t) slice << ") not in list of subscriptions: BBFrame dropped");
    }
}

void
SatUtPhy::UpdateSliceSubscription(uint8_t slice)
{
  if (slice == 0)
    {
      m_slicesSubscribed.clear ();
    }
  else
    {
      m_slicesSubscribed.insert (slice);
    }
}

} // namespace ns3
