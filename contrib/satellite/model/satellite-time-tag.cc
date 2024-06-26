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

#include "satellite-time-tag.h"

namespace ns3 {

/*
 * There are 4 classes defined here: SatTimeTag, SatPhyTimeTag, SatMacTimeTag,
 * SatDevTimeTag, and SatAppTimeTag. Except of the name difference, they share
 * exactly the same definitions.
 */

// LLC ////////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatTimeTag);

SatTimeTag::SatTimeTag ()
  : m_senderTimestamp (Seconds (0))
{
  // Nothing to do here
}

SatTimeTag::SatTimeTag (Time senderTimestamp)
  : m_senderTimestamp (senderTimestamp)
{
  // Nothing to do here
}

TypeId
SatTimeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatTimeTag")
    .SetParent<Tag> ()
    .AddConstructor<SatTimeTag> ();
  return tid;
}

TypeId
SatTimeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatTimeTag::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
SatTimeTag::Serialize (TagBuffer i) const
{
  int64_t senderTimestamp = m_senderTimestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&senderTimestamp, sizeof(int64_t));
}

void
SatTimeTag::Deserialize (TagBuffer i)
{
  int64_t senderTimestamp;
  i.Read ((uint8_t *)&senderTimestamp, 8);
  m_senderTimestamp = NanoSeconds (senderTimestamp);
}

void
SatTimeTag::Print (std::ostream &os) const
{
  os << m_senderTimestamp;
}

Time
SatTimeTag::GetSenderTimestamp (void) const
{
  return m_senderTimestamp;
}

void
SatTimeTag::SetSenderTimestamp (Time senderTimestamp)
{
  this->m_senderTimestamp = senderTimestamp;
}


// PHY ////////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatPhyTimeTag);

SatPhyTimeTag::SatPhyTimeTag ()
  : m_senderTimestamp (Seconds (0))
{
  // Nothing to do here
}

SatPhyTimeTag::SatPhyTimeTag (Time senderTimestamp)
  : m_senderTimestamp (senderTimestamp)
{
  // Nothing to do here
}

TypeId
SatPhyTimeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyTimeTag")
    .SetParent<Tag> ()
    .AddConstructor<SatPhyTimeTag> ();
  return tid;
}

TypeId
SatPhyTimeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatPhyTimeTag::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
SatPhyTimeTag::Serialize (TagBuffer i) const
{
  int64_t senderTimestamp = m_senderTimestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&senderTimestamp, sizeof(int64_t));
}

void
SatPhyTimeTag::Deserialize (TagBuffer i)
{
  int64_t senderTimestamp;
  i.Read ((uint8_t *)&senderTimestamp, 8);
  m_senderTimestamp = NanoSeconds (senderTimestamp);
}

void
SatPhyTimeTag::Print (std::ostream &os) const
{
  os << m_senderTimestamp;
}

Time
SatPhyTimeTag::GetSenderTimestamp (void) const
{
  return m_senderTimestamp;
}

void
SatPhyTimeTag::SetSenderTimestamp (Time senderTimestamp)
{
  this->m_senderTimestamp = senderTimestamp;
}


// PHY LINK ////////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatPhyLinkTimeTag);

SatPhyLinkTimeTag::SatPhyLinkTimeTag ()
  : m_senderLinkTimestamp (Seconds (0))
{
  // Nothing to do here
}

SatPhyLinkTimeTag::SatPhyLinkTimeTag (Time senderLinkTimestamp)
  : m_senderLinkTimestamp (senderLinkTimestamp)
{
  // Nothing to do here
}

TypeId
SatPhyLinkTimeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyLinkTimeTag")
    .SetParent<Tag> ()
    .AddConstructor<SatPhyLinkTimeTag> ();
  return tid;
}

TypeId
SatPhyLinkTimeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatPhyLinkTimeTag::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
SatPhyLinkTimeTag::Serialize (TagBuffer i) const
{
  int64_t senderTimestamp = m_senderLinkTimestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&senderTimestamp, sizeof(int64_t));
}

void
SatPhyLinkTimeTag::Deserialize (TagBuffer i)
{
  int64_t senderTimestamp;
  i.Read ((uint8_t *)&senderTimestamp, 8);
  m_senderLinkTimestamp = NanoSeconds (senderTimestamp);
}

void
SatPhyLinkTimeTag::Print (std::ostream &os) const
{
  os << m_senderLinkTimestamp;
}

Time
SatPhyLinkTimeTag::GetSenderLinkTimestamp (void) const
{
  return m_senderLinkTimestamp;
}

void
SatPhyLinkTimeTag::SetSenderLinkTimestamp (Time senderLinkTimestamp)
{
  this->m_senderLinkTimestamp = senderLinkTimestamp;
}


// MAC ////////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatMacTimeTag);

SatMacTimeTag::SatMacTimeTag ()
  : m_senderTimestamp (Seconds (0))
{
  // Nothing to do here
}

SatMacTimeTag::SatMacTimeTag (Time senderTimestamp)
  : m_senderTimestamp (senderTimestamp)
{
  // Nothing to do here
}

TypeId
SatMacTimeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMacTimeTag")
    .SetParent<Tag> ()
    .AddConstructor<SatMacTimeTag> ();
  return tid;
}

TypeId
SatMacTimeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatMacTimeTag::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
SatMacTimeTag::Serialize (TagBuffer i) const
{
  int64_t senderTimestamp = m_senderTimestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&senderTimestamp, sizeof(int64_t));
}

void
SatMacTimeTag::Deserialize (TagBuffer i)
{
  int64_t senderTimestamp;
  i.Read ((uint8_t *)&senderTimestamp, 8);
  m_senderTimestamp = NanoSeconds (senderTimestamp);
}

void
SatMacTimeTag::Print (std::ostream &os) const
{
  os << m_senderTimestamp;
}

Time
SatMacTimeTag::GetSenderTimestamp (void) const
{
  return m_senderTimestamp;
}

void
SatMacTimeTag::SetSenderTimestamp (Time senderTimestamp)
{
  this->m_senderTimestamp = senderTimestamp;
}


// MAC LINK ////////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatMacLinkTimeTag);

SatMacLinkTimeTag::SatMacLinkTimeTag ()
  : m_senderLinkTimestamp (Seconds (0))
{
  // Nothing to do here
}

SatMacLinkTimeTag::SatMacLinkTimeTag (Time senderLinkTimestamp)
  : m_senderLinkTimestamp (senderLinkTimestamp)
{
  // Nothing to do here
}

TypeId
SatMacLinkTimeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMacLinkTimeTag")
    .SetParent<Tag> ()
    .AddConstructor<SatMacLinkTimeTag> ();
  return tid;
}

TypeId
SatMacLinkTimeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatMacLinkTimeTag::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
SatMacLinkTimeTag::Serialize (TagBuffer i) const
{
  int64_t senderTimestamp = m_senderLinkTimestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&senderTimestamp, sizeof(int64_t));
}

void
SatMacLinkTimeTag::Deserialize (TagBuffer i)
{
  int64_t senderTimestamp;
  i.Read ((uint8_t *)&senderTimestamp, 8);
  m_senderLinkTimestamp = NanoSeconds (senderTimestamp);
}

void
SatMacLinkTimeTag::Print (std::ostream &os) const
{
  os << m_senderLinkTimestamp;
}

Time
SatMacLinkTimeTag::GetSenderLinkTimestamp (void) const
{
  return m_senderLinkTimestamp;
}

void
SatMacLinkTimeTag::SetSenderLinkTimestamp (Time senderLinkTimestamp)
{
  this->m_senderLinkTimestamp = senderLinkTimestamp;
}


// DEV ////////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatDevTimeTag);

SatDevTimeTag::SatDevTimeTag ()
  : m_senderTimestamp (Seconds (0))
{
  // Nothing to do here
}

SatDevTimeTag::SatDevTimeTag (Time senderTimestamp)
  : m_senderTimestamp (senderTimestamp)
{
  // Nothing to do here
}

TypeId
SatDevTimeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatDevTimeTag")
    .SetParent<Tag> ()
    .AddConstructor<SatDevTimeTag> ();
  return tid;
}

TypeId
SatDevTimeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatDevTimeTag::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
SatDevTimeTag::Serialize (TagBuffer i) const
{
  int64_t senderTimestamp = m_senderTimestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&senderTimestamp, sizeof(int64_t));
}

void
SatDevTimeTag::Deserialize (TagBuffer i)
{
  int64_t senderTimestamp;
  i.Read ((uint8_t *)&senderTimestamp, 8);
  m_senderTimestamp = NanoSeconds (senderTimestamp);
}

void
SatDevTimeTag::Print (std::ostream &os) const
{
  os << m_senderTimestamp;
}

Time
SatDevTimeTag::GetSenderTimestamp (void) const
{
  return m_senderTimestamp;
}

void
SatDevTimeTag::SetSenderTimestamp (Time senderTimestamp)
{
  this->m_senderTimestamp = senderTimestamp;
}


// DEV LINK ////////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatDevLinkTimeTag);

SatDevLinkTimeTag::SatDevLinkTimeTag ()
  : m_senderTimestamp (Seconds (0))
{
  // Nothing to do here
}

SatDevLinkTimeTag::SatDevLinkTimeTag (Time senderTimestamp)
  : m_senderTimestamp (senderTimestamp)
{
  // Nothing to do here
}

TypeId
SatDevLinkTimeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatDevLinkTimeTag")
    .SetParent<Tag> ()
    .AddConstructor<SatDevLinkTimeTag> ();
  return tid;
}

TypeId
SatDevLinkTimeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatDevLinkTimeTag::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
SatDevLinkTimeTag::Serialize (TagBuffer i) const
{
  int64_t senderTimestamp = m_senderTimestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&senderTimestamp, sizeof(int64_t));
}

void
SatDevLinkTimeTag::Deserialize (TagBuffer i)
{
  int64_t senderTimestamp;
  i.Read ((uint8_t *)&senderTimestamp, 8);
  m_senderTimestamp = NanoSeconds (senderTimestamp);
}

void
SatDevLinkTimeTag::Print (std::ostream &os) const
{
  os << m_senderTimestamp;
}

Time
SatDevLinkTimeTag::GetSenderTimestamp (void) const
{
  return m_senderTimestamp;
}

void
SatDevLinkTimeTag::SetSenderTimestamp (Time senderTimestamp)
{
  this->m_senderTimestamp = senderTimestamp;
}


} // namespace ns3

