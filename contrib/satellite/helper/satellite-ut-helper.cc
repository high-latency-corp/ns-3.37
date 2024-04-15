/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 * Author: Mathias Ettinger <mettinger@viveris.toulouse.fr>
 */

#include <ns3/config.h>
#include <ns3/log.h>
#include <ns3/names.h>
#include <ns3/enum.h>
#include <ns3/double.h>
#include <ns3/pointer.h>
#include <ns3/uinteger.h>
#include <ns3/string.h>
#include <ns3/callback.h>
#include <ns3/config.h>
#include <ns3/nstime.h>
#include <ns3/singleton.h>

#include <ns3/satellite-const-variables.h>
#include <ns3/satellite-utils.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-mobility-observer.h>
#include <ns3/satellite-gw-llc.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-lorawan-net-device.h>
#include <ns3/satellite-ut-llc.h>
#include <ns3/satellite-ut-mac.h>
#include <ns3/satellite-ut-handover-module.h>
#include <ns3/satellite-ut-phy.h>
#include <ns3/satellite-phy-tx.h>
#include <ns3/satellite-phy-rx.h>
#include <ns3/satellite-phy-rx-carrier-conf.h>
#include <ns3/satellite-base-encapsulator.h>
#include <ns3/satellite-generic-stream-encapsulator.h>
#include <ns3/satellite-generic-stream-encapsulator-arq.h>
#include <ns3/satellite-return-link-encapsulator.h>
#include <ns3/satellite-return-link-encapsulator-arq.h>
#include <ns3/satellite-node-info.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-request-manager.h>
#include <ns3/satellite-queue.h>
#include <ns3/satellite-ut-scheduler.h>
#include <ns3/satellite-channel-estimation-error-container.h>
#include <ns3/satellite-packet-classifier.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-typedefs.h>
#include <ns3/satellite-mac-tag.h>

#include <ns3/satellite-lora-conf.h>
#include <ns3/lorawan-mac-end-device-class-a.h>

#include "satellite-ut-helper.h"


NS_LOG_COMPONENT_DEFINE ("SatUtHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUtHelper);

TypeId
SatUtHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatUtHelper")
    .SetParent<Object> ()
    .AddConstructor<SatUtHelper> ()
    .AddAttribute ("DaFwdLinkInterferenceModel",
                   "Forward link interference model for dedicated access",
                   EnumValue (SatPhyRxCarrierConf::IF_CONSTANT),
                   MakeEnumAccessor (&SatUtHelper::m_daInterferenceModel),
                   MakeEnumChecker (SatPhyRxCarrierConf::IF_CONSTANT, "Constant",
                                    SatPhyRxCarrierConf::IF_TRACE, "Trace",
                                    SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket",
                                    SatPhyRxCarrierConf::IF_PER_FRAGMENT, "PerFragment"))
    .AddAttribute ("FwdLinkErrorModel",
                   "Forward link error model",
                   EnumValue (SatPhyRxCarrierConf::EM_AVI),
                   MakeEnumAccessor (&SatUtHelper::m_errorModel),
                   MakeEnumChecker (SatPhyRxCarrierConf::EM_NONE, "None",
                                    SatPhyRxCarrierConf::EM_CONSTANT, "Constant",
                                    SatPhyRxCarrierConf::EM_AVI, "AVI"))
    .AddAttribute ("FwdLinkConstantErrorRate",
                   "Constant error rate",
                   DoubleValue (0.01),
                   MakeDoubleAccessor (&SatUtHelper::m_daConstantErrorRate),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("LowerLayerServiceConf",
                   "Pointer to lower layer service configuration.",
                   PointerValue (),
                   MakePointerAccessor (&SatUtHelper::m_llsConf),
                   MakePointerChecker<SatLowerLayerServiceConf> ())
    .AddAttribute ("EnableChannelEstimationError",
                   "Enable channel estimation error in forward link receiver at UT.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SatUtHelper::m_enableChannelEstimationError),
                   MakeBooleanChecker ())
    .AddAttribute ("UseCrdsaOnlyForControlPackets",
                   "CRDSA utilized only for control packets or also for user data.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatUtHelper::m_crdsaOnlyForControl),
                   MakeBooleanChecker ())
    .AddAttribute ("AsynchronousReturnAccess",
                   "Use asynchronous access methods on the return channel.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatUtHelper::m_asyncAccess),
                   MakeBooleanChecker ())
    .AddTraceSource ("Creation",
                     "Creation traces",
                     MakeTraceSourceAccessor (&SatUtHelper::m_creationTrace),
                     "ns3::SatTypedefs::CreationCallback")
  ;
  return tid;
}

TypeId
SatUtHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatUtHelper::SatUtHelper ()
  : m_carrierBandwidthConverter (),
  m_fwdLinkCarrierCount (),
  m_superframeSeq (),
  m_daInterferenceModel (SatPhyRxCarrierConf::IF_CONSTANT),
  m_errorModel (SatPhyRxCarrierConf::EM_AVI),
  m_daConstantErrorRate (0.0),
  m_linkResults (),
  m_llsConf (),
  m_enableChannelEstimationError (false),
  m_crdsaOnlyForControl (false),
  m_asyncAccess (false),
  m_raSettings ()
{
  NS_LOG_FUNCTION (this);

  // this default constructor should be never called
  NS_FATAL_ERROR ("SatUtHelper::SatUtHelper - Constructor not in use");
}

SatUtHelper::SatUtHelper (SatTypedefs::CarrierBandwidthConverter_t carrierBandwidthConverter,
                          uint32_t fwdLinkCarrierCount,
                          Ptr<SatSuperframeSeq> seq,
                          SatMac::ReadCtrlMsgCallback readCb,
                          SatMac::ReserveCtrlMsgCallback reserveCb,
                          SatMac::SendCtrlMsgCallback sendCb,
                          RandomAccessSettings_s randomAccessSettings)
  : m_carrierBandwidthConverter (carrierBandwidthConverter),
  m_fwdLinkCarrierCount (fwdLinkCarrierCount),
  m_superframeSeq (seq),
  m_readCtrlCb (readCb),
  m_reserveCtrlCb (reserveCb),
  m_sendCtrlCb (sendCb),
  m_daInterferenceModel (SatPhyRxCarrierConf::IF_CONSTANT),
  m_errorModel (SatPhyRxCarrierConf::EM_AVI),
  m_daConstantErrorRate (0.0),
  m_linkResults (),
  m_llsConf (),
  m_enableChannelEstimationError (false),
  m_crdsaOnlyForControl (false),
  m_raSettings (randomAccessSettings)
{
  NS_LOG_FUNCTION (this << fwdLinkCarrierCount << seq );
  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  m_channelFactory.SetTypeId ("ns3::SatChannel");

  m_llsConf = CreateObject<SatLowerLayerServiceConf>  ();
}

void
SatUtHelper::Initialize (Ptr<SatLinkResultsFwd> lrFwd)
{
  NS_LOG_FUNCTION (this);
  /*
   * Forward channel link results (DVB-S2 or DVB-S2X) are created for UTs.
   */
  if (lrFwd && m_errorModel == SatPhyRxCarrierConf::EM_AVI)
    {
      m_linkResults = lrFwd;
    }
}

void
SatUtHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_deviceFactory.Set (n1, v1);
}

void
SatUtHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_channelFactory.Set (n1, v1);
}

void
SatUtHelper::SetPhyAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  Config::SetDefault ("ns3::SatUtPhy::" + n1, v1);
}

NetDeviceContainer
SatUtHelper::InstallDvb (NodeContainer c, uint32_t satId, uint32_t beamId,
                         Ptr<SatChannel> fCh, Ptr<SatChannel> rCh,
                         Ptr<SatNetDevice> gwNd, Ptr<SatNcc> ncc,
                         Address satUserAddress,
                         SatPhy::ChannelPairGetterCallback cbChannel,
                         SatMac::RoutingUpdateCallback cbRouting,
                         SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                          SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
  NS_LOG_FUNCTION (this << beamId << fCh << rCh );

  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      devs.Add (InstallDvb (*i, satId, beamId, fCh, rCh, gwNd, ncc, satUserAddress, cbChannel, cbRouting, forwardLinkRegenerationMode, returnLinkRegenerationMode));
    }

  return devs;
}

Ptr<NetDevice>
SatUtHelper::InstallDvb (Ptr<Node> n, uint32_t satId, uint32_t beamId,
                         Ptr<SatChannel> fCh, Ptr<SatChannel> rCh,
                         Ptr<SatNetDevice> gwNd, Ptr<SatNcc> ncc,
                         Address satUserAddress,
                         SatPhy::ChannelPairGetterCallback cbChannel,
                         SatMac::RoutingUpdateCallback cbRouting,
                         SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                         SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
  NS_LOG_FUNCTION (this << n << beamId << fCh << rCh );

  NetDeviceContainer container;

  // Create SatNetDevice
  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  Ptr<SatNetDevice> dev = m_deviceFactory.Create<SatNetDevice> ();

  // Attach the SatNetDevice to node
  n->AddDevice (dev);

  SatPhy::CreateParam_t params;
  params.m_satId = satId;
  params.m_beamId = beamId;
  params.m_device = dev;
  params.m_txCh = rCh;
  params.m_rxCh = fCh;
  params.m_standard = SatEnums::DVB_UT;

  // Create a packet classifier
  Ptr<SatPacketClassifier> classifier = Create<SatPacketClassifier> ();

  /**
   * Channel estimation errors
   */
  Ptr<SatChannelEstimationErrorContainer> cec;
  // Not enabled, create only base class
  if (!m_enableChannelEstimationError)
    {
      cec = Create<SatSimpleChannelEstimationErrorContainer> ();
    }
  // Create SatFwdLinkChannelEstimationErrorContainer
  else
    {
      cec = Create<SatFwdLinkChannelEstimationErrorContainer> ();
    }

  SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters = SatPhyRxCarrierConf::RxCarrierCreateParams_s ();
  parameters.m_errorModel = m_errorModel;
  parameters.m_daConstantErrorRate = m_daConstantErrorRate;
  parameters.m_daIfModel = m_daInterferenceModel;
  parameters.m_raIfModel = m_raSettings.m_raInterferenceModel;
  parameters.m_raIfEliminateModel = m_raSettings.m_raInterferenceEliminationModel;
  parameters.m_linkRegenerationMode = forwardLinkRegenerationMode;
  parameters.m_bwConverter = m_carrierBandwidthConverter;
  parameters.m_carrierCount = m_fwdLinkCarrierCount;
  parameters.m_cec = cec;
  parameters.m_raCollisionModel = m_raSettings.m_raCollisionModel;
  parameters.m_randomAccessModel = m_raSettings.m_randomAccessModel;

  Ptr<SatUtPhy> phy = CreateObject<SatUtPhy> (params,
                                              m_linkResults,
                                              parameters,
                                              m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE),
                                              forwardLinkRegenerationMode);
  phy->SetChannelPairGetterCallback (cbChannel);

  // Set fading
  phy->SetTxFadingContainer (n->GetObject<SatBaseFading> ());
  phy->SetRxFadingContainer (n->GetObject<SatBaseFading> ());

  Ptr<SatUtMac> mac = CreateObject<SatUtMac> (satId,
                                              beamId,
                                              m_superframeSeq,
                                              forwardLinkRegenerationMode,
                                              returnLinkRegenerationMode,
                                              m_crdsaOnlyForControl);

  // Set the control message container callbacks
  mac->SetReadCtrlCallback (m_readCtrlCb);
  mac->SetReserveCtrlCallback (m_reserveCtrlCb);
  mac->SetSendCtrlCallback (m_sendCtrlCb);

  // Set timing advance callback to mac (if not asynchronous access)
  if (m_raSettings.m_randomAccessModel != SatEnums::RA_MODEL_ESSA)
    {
      Ptr<SatMobilityObserver> observer = n->GetObject<SatMobilityObserver> ();
      NS_ASSERT (observer != NULL);

      SatUtMac::TimingAdvanceCallback timingCb = MakeCallback (&SatMobilityObserver::GetTimingAdvance, observer);
      mac->SetTimingAdvanceCallback (timingCb);
    }

  // Attach the Mac layer receiver to Phy
  SatPhy::ReceiveCallback recCb = MakeCallback (&SatUtMac::Receive, mac);

  phy->SetAttribute ("ReceiveCb", CallbackValue (recCb));
  mac->SetTxCheckCallback (MakeCallback (&SatUtPhy::IsTxPossible, phy));

  // Create callback to inform phy layer slices subscription
  mac->SetSliceSubscriptionCallback (MakeCallback (&SatUtPhy::UpdateSliceSubscription, phy));

  // Create Logical Link Control (LLC) layer
  Ptr<SatUtLlc> llc = CreateObject<SatUtLlc> (forwardLinkRegenerationMode,
                                              returnLinkRegenerationMode);

  // Set the control msg read callback to LLC due to ARQ ACKs
  llc->SetReadCtrlCallback (m_readCtrlCb);

  // Create a request manager and attach it to LLC, and set control message callback to RM
  Ptr<SatRequestManager> rm = CreateObject<SatRequestManager> ();
  llc->SetRequestManager (rm);
  rm->SetCtrlMsgCallback (MakeCallback (&SatNetDevice::SendControlMsg, dev));

  if (returnLinkRegenerationMode != SatEnums::TRANSPARENT && returnLinkRegenerationMode != SatEnums::REGENERATION_PHY)
    {
      llc->SetAdditionalHeaderSize (SatAddressE2ETag::SIZE);
      rm->SetHeaderOffsetVbdc(38.0/(38-2-SatAddressE2ETag::SIZE));
    }

  // Set the callback to check whether control msg transmissions are possible
  rm->SetCtrlMsgTxPossibleCallback (MakeCallback (&SatUtMac::ControlMsgTransmissionPossible, mac));

  // Set the callback to check whether logon msg transmissions are possible
  rm->SetLogonMsgTxPossibleCallback (MakeCallback (&SatUtMac::LogonMsgTransmissionPossible, mac));

  // Set TBTP callback to UT MAC
  mac->SetAssignedDaResourcesCallback (MakeCallback (&SatRequestManager::AssignedDaResources, rm));

  // Set Send Logon callback to UT MAC
  mac->SetSendLogonCallback (MakeCallback (&SatRequestManager::SendLogonMessage, rm));

  // Attach the PHY layer to SatNetDevice
  dev->SetPhy (phy);

  // Attach the Mac layer to SatNetDevice
  dev->SetMac (mac);

  // Attach the LLC layer to SatNetDevice
  dev->SetLlc (llc);

  // Attach the packet classifier
  dev->SetPacketClassifier (classifier);

  // Attach the Mac layer C/N0 updates receiver to Phy
  SatPhy::CnoCallback cnoCb = MakeCallback (&SatRequestManager::CnoUpdated, rm);
  phy->SetAttribute ("CnoCb", CallbackValue (cnoCb));

  // Set the device address and pass it to MAC as well
  Mac48Address addr = Mac48Address::Allocate ();
  dev->SetAddress (addr);

  Singleton<SatIdMapper>::Get ()->AttachMacToTraceId (dev->GetAddress ());
  Singleton<SatIdMapper>::Get ()->AttachMacToUtId (dev->GetAddress ());
  Singleton<SatIdMapper>::Get ()->AttachMacToBeamId (dev->GetAddress (), beamId);
  Singleton<SatIdMapper>::Get ()->AttachMacToSatId (dev->GetAddress (), satId+1);

  // Create encapsulator and add it to UT's LLC
  Mac48Address gwAddr = Mac48Address::ConvertFrom (gwNd->GetAddress ());

  // Create an encapsulator for control messages.
  // Source = UT MAC address
  // Destination = GW MAC address (or SAT user MAC address if regenerative)
  // Flow id = by default 0
  Ptr<SatBaseEncapsulator> utEncap;
  if (returnLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
      utEncap = CreateObject<SatBaseEncapsulator> (addr, Mac48Address::ConvertFrom (satUserAddress), addr, gwAddr, SatEnums::CONTROL_FID);
    }
  else
    {
      utEncap = CreateObject<SatBaseEncapsulator> (addr, gwAddr, addr, gwAddr, SatEnums::CONTROL_FID);
    }

  // Create queue event callbacks to MAC (for random access) and RM (for on-demand DAMA)
  SatQueue::QueueEventCallback macCb;
  if (m_raSettings.m_randomAccessModel == SatEnums::RA_MODEL_ESSA)
    {
      macCb = MakeCallback (&SatUtMac::ReceiveQueueEventEssa, mac);
    }
  else
    {
      macCb = MakeCallback (&SatUtMac::ReceiveQueueEvent, mac);
    }
  SatQueue::LogonCallback logonCb = MakeCallback (&SatUtMac::SendLogon, mac);
  SatQueue::QueueEventCallback rmCb = MakeCallback (&SatRequestManager::ReceiveQueueEvent, rm);

  // Create a queue
  Ptr<SatQueue> queue = CreateObject<SatQueue> (SatEnums::CONTROL_FID);
  queue->AddLogonCallback (logonCb);
  queue->AddQueueEventCallback (macCb);
  queue->AddQueueEventCallback (rmCb);
  utEncap->SetQueue (queue);
  if (returnLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
      llc->AddEncap (addr, Mac48Address::ConvertFrom (satUserAddress), SatEnums::CONTROL_FID, utEncap);
    }
  else
    {
      llc->AddEncap (addr, gwAddr, SatEnums::CONTROL_FID, utEncap);
    }
  rm->AddQueueCallback (SatEnums::CONTROL_FID, MakeCallback (&SatQueue::GetQueueStatistics, queue));

  // Add callbacks to LLC for future need. LLC creates encapsulators and
  // decapsulators dynamically 'on-a-need-basis'.
  llc->SetCtrlMsgCallback (MakeCallback (&SatNetDevice::SendControlMsg, dev));
  llc->SetMacQueueEventCallback (macCb);

  // set serving GW MAC address to RM
  mac->SetRoutingUpdateCallback (cbRouting);
  mac->SetGatewayUpdateCallback (MakeCallback (&SatUtLlc::SetGwAddress, llc));
  mac->SetGwAddress (gwAddr);

  // Attach the transmit callback to PHY
  mac->SetTransmitCallback (MakeCallback (&SatPhy::SendPdu, phy));

  // Attach the PHY handover callback to SatMac
  mac->SetHandoverCallback (MakeCallback (&SatUtPhy::PerformHandover, phy));

  // Attach the LLC receive callback to SatMac
  mac->SetReceiveCallback (MakeCallback (&SatLlc::Receive, llc));

  llc->SetReceiveCallback (MakeCallback (&SatNetDevice::Receive, dev));

  Ptr<SatSuperframeConf> superFrameConf = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE);
  bool enableLogon = superFrameConf->IsLogonEnabled ();
  uint32_t logonChannelId = superFrameConf->GetLogonChannelIndex ();

  // Add UT to NCC
  if (enableLogon)
    {
      ncc->ReserveLogonChannel (logonChannelId);
    }
  else
    {
      ncc->AddUt (m_llsConf, dev->GetAddress (), satId, beamId, MakeCallback (&SatUtMac::SetRaChannel, mac));
    }

  phy->Initialize ();

  // Create UT scheduler for MAC and connect callbacks to LLC
  Ptr<SatUtScheduler> utScheduler = CreateObject<SatUtScheduler> (m_llsConf);
  utScheduler->SetTxOpportunityCallback (MakeCallback (&SatUtLlc::NotifyTxOpportunity, llc));
  utScheduler->SetSchedContextCallback (MakeCallback (&SatLlc::GetSchedulingContexts, llc));
  mac->SetAttribute ("Scheduler", PointerValue (utScheduler));

  // Create a node info to all the protocol layers
  Ptr<SatNodeInfo> nodeInfo = Create <SatNodeInfo> (SatEnums::NT_UT, n->GetId (), addr);
  dev->SetNodeInfo (nodeInfo);
  llc->SetNodeInfo (nodeInfo);
  mac->SetNodeInfo (nodeInfo);
  phy->SetNodeInfo (nodeInfo);

  rm->Initialize (m_llsConf, m_superframeSeq->GetDuration (0));

  if (m_raSettings.m_randomAccessModel != SatEnums::RA_MODEL_OFF)
    {
      Ptr<SatRandomAccessConf> randomAccessConf = CreateObject<SatRandomAccessConf> (m_llsConf, m_superframeSeq);

      /// create RA module with defaults
      Ptr<SatRandomAccess> randomAccess = CreateObject<SatRandomAccess> (randomAccessConf, m_raSettings.m_randomAccessModel);

      /// attach callbacks
      if (m_crdsaOnlyForControl)
        {
          randomAccess->SetAreBuffersEmptyCallback (MakeCallback (&SatLlc::ControlBuffersEmpty, llc));
        }
      else
        {
          randomAccess->SetAreBuffersEmptyCallback (MakeCallback (&SatLlc::BuffersEmpty, llc));
        }

      /// attach the RA module
      mac->SetRandomAccess (randomAccess);
      if (enableLogon)
        {
          mac->SetLogonChannel (logonChannelId);
          mac->LogOff ();
        }
    }
  else if (enableLogon)
    {
      NS_FATAL_ERROR ("Cannot simulate logon without a RA frame");
    }

  Ptr<SatUtHandoverModule> utHandoverModule = n->GetObject<SatUtHandoverModule> ();
  if (utHandoverModule != NULL)
    {
      utHandoverModule->SetHandoverRequestCallback (MakeCallback (&SatRequestManager::SendHandoverRecommendation, rm));
      mac->SetBeamCheckerCallback (MakeCallback (&SatUtHandoverModule::CheckForHandoverRecommendation, utHandoverModule));
      mac->SetAskedBeamCallback (MakeCallback (&SatUtHandoverModule::GetAskedBeamId, utHandoverModule));
      mac->SetBeamScheculerCallback (MakeCallback (&SatNcc::GetBeamScheduler, ncc));
      mac->SetUpdateGwAddressCallback (MakeCallback (&SatRequestManager::SetGwAddress, rm));
    }

  return dev;
}

NetDeviceContainer
SatUtHelper::InstallLora (NodeContainer c, uint32_t satId, uint32_t beamId,
                          Ptr<SatChannel> fCh, Ptr<SatChannel> rCh,
                          Ptr<SatNetDevice> gwNd, Ptr<SatNcc> ncc,
                          Address satUserAddress,
                          SatPhy::ChannelPairGetterCallback cbChannel,
                          SatMac::RoutingUpdateCallback cbRouting,
                          SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                          SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
  NS_LOG_FUNCTION (this << beamId << fCh << rCh );

  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      devs.Add (InstallLora (*i, satId, beamId, fCh, rCh, gwNd, ncc, satUserAddress, cbChannel, cbRouting, forwardLinkRegenerationMode, returnLinkRegenerationMode));
    }

  return devs;
}

Ptr<NetDevice>
SatUtHelper::InstallLora (Ptr<Node> n, uint32_t satId, uint32_t beamId,
                          Ptr<SatChannel> fCh, Ptr<SatChannel> rCh,
                          Ptr<SatNetDevice> gwNd, Ptr<SatNcc> ncc,
                          Address satUserAddress,
                          SatPhy::ChannelPairGetterCallback cbChannel,
                          SatMac::RoutingUpdateCallback cbRouting,
                          SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                          SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
  NS_LOG_FUNCTION (this << n << beamId << fCh << rCh );

  NetDeviceContainer container;

  // Create SatNetDevice
  m_deviceFactory.SetTypeId ("ns3::SatLorawanNetDevice");
  Ptr<SatLorawanNetDevice> dev = m_deviceFactory.Create<SatLorawanNetDevice> ();

  // Attach the SatNetDevice to node
  n->AddDevice (dev);

  SatPhy::CreateParam_t params;
  params.m_satId = satId;
  params.m_beamId = beamId;
  params.m_device = dev;
  params.m_txCh = rCh;
  params.m_rxCh = fCh;
  params.m_standard = SatEnums::LORA_UT;

  /**
   * Channel estimation errors
   */
  Ptr<SatChannelEstimationErrorContainer> cec;
  // Not enabled, create only base class
  if (!m_enableChannelEstimationError)
    {
      cec = Create<SatSimpleChannelEstimationErrorContainer> ();
    }
  // Create SatFwdLinkChannelEstimationErrorContainer
  else
    {
      cec = Create<SatFwdLinkChannelEstimationErrorContainer> ();
    }

  SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters = SatPhyRxCarrierConf::RxCarrierCreateParams_s ();
  parameters.m_errorModel = m_errorModel;
  parameters.m_daConstantErrorRate = m_daConstantErrorRate;
  parameters.m_daIfModel = m_daInterferenceModel;
  parameters.m_raIfModel = m_raSettings.m_raInterferenceModel;
  parameters.m_raIfEliminateModel = m_raSettings.m_raInterferenceEliminationModel;
  parameters.m_linkRegenerationMode = forwardLinkRegenerationMode;
  parameters.m_bwConverter = m_carrierBandwidthConverter;
  parameters.m_carrierCount = m_fwdLinkCarrierCount;
  parameters.m_cec = cec;
  parameters.m_raCollisionModel = m_raSettings.m_raCollisionModel;
  parameters.m_randomAccessModel = m_raSettings.m_randomAccessModel;

  Ptr<SatUtPhy> phy = CreateObject<SatUtPhy> (params,
                                              m_linkResults,
                                              parameters,
                                              m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE),
                                              forwardLinkRegenerationMode);
  phy->SetChannelPairGetterCallback (cbChannel);

  // Set fading
  phy->SetTxFadingContainer (n->GetObject<SatBaseFading> ());
  phy->SetRxFadingContainer (n->GetObject<SatBaseFading> ());

  Ptr<LorawanMacEndDeviceClassA> mac = CreateObject<LorawanMacEndDeviceClassA> (satId, beamId, m_superframeSeq);

  // TODO configuration for EU only
  mac->SetTxDbmForTxPower (std::vector<double>{16, 14, 12, 10, 8, 6, 4, 2});

  SatLoraConf satLoraConf;
  satLoraConf.SetConf (mac);

  // Attach the Mac layer receiver to Phy
  SatPhy::ReceiveCallback recCb = MakeCallback (&LorawanMac::Receive, mac);

  phy->SetAttribute ("ReceiveCb", CallbackValue (recCb));

  // Attach the PHY layer to SatNetDevice
  dev->SetPhy (phy);

  // Attach the Mac layer to SatNetDevice
  dev->SetLorawanMac (mac);
  mac->SetDevice (dev);

  mac->SetPhy (phy);
  mac->SetPhyRx (DynamicCast<SatLoraPhyRx> (phy->GetPhyRx ()));
  mac->SetRaModel (m_raSettings.m_randomAccessModel);

  // Set the device address and pass it to MAC as well
  Mac48Address addr = Mac48Address::Allocate ();
  dev->SetAddress (addr);

  Singleton<SatIdMapper>::Get ()->AttachMacToTraceId (dev->GetAddress ());
  Singleton<SatIdMapper>::Get ()->AttachMacToUtId (dev->GetAddress ());
  Singleton<SatIdMapper>::Get ()->AttachMacToBeamId (dev->GetAddress (), beamId);
  Singleton<SatIdMapper>::Get ()->AttachMacToSatId (dev->GetAddress (), satId+1);

  // Create encapsulator and add it to UT's LLC
  Mac48Address gwAddr = Mac48Address::ConvertFrom (gwNd->GetAddress ());

  // set serving GW MAC address to RM
  mac->SetRoutingUpdateCallback (cbRouting);
  mac->SetGwAddress (gwAddr);

  // Add UT to NCC
  ncc->AddUt (m_llsConf, dev->GetAddress (), satId, beamId, MakeCallback (&LorawanMacEndDevice::SetRaChannel, mac));

  phy->Initialize ();

  // Create a node info to all the protocol layers
  Ptr<SatNodeInfo> nodeInfo = Create <SatNodeInfo> (SatEnums::NT_UT, n->GetId (), addr);
  dev->SetNodeInfo (nodeInfo);
  mac->SetNodeInfo (nodeInfo);
  phy->SetNodeInfo (nodeInfo);

  return dev;
}

void
SatUtHelper::EnableCreationTraces (Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this);

  TraceConnect ("Creation", "SatUtHelper", cb);
}

} // namespace ns3
