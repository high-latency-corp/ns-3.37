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

#ifndef CBR_HELPER_H
#define CBR_HELPER_H

#include "ns3/address.h"
#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include <stdint.h>
#include <string>

namespace ns3
{

class DataRate;

/**
 * \brief A helper to make it easier to instantiate an ns3::CbrApplication
 * on a set of nodes.
 */
class CbrHelper
{
  public:
    /**
     * Create an CbrHelper to make it easier to work with CbrApplications
     *
     * \param protocol the name of the protocol to use to send traffic
     *        by the applications. This string identifies the socket
     *        factory type used to create sockets for the applications.
     *        A typical value would be ns3::UdpSocketFactory.
     * \param address the address of the remote node to send traffic
     *        to.
     */
    CbrHelper(std::string protocol, Address address);

    /**
     * Helper function used to set the underlying application attributes.
     *
     * \param name the name of the application attribute to set
     * \param value the value of the application attribute to set
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * Helper function to set a constant constant traffic. Equivalent to
     * setting the attributes Interval and DataRate accordingly.
     *
     * \param interval Interval for the sending packets
     * \param packetSize size in bytes of the packet payloads generated
     */
    void SetConstantTraffic(Time interval, uint32_t packetSize = 512);

    /**
     * Install an ns3::CbrApplication on each node of the input container
     * configured with all the attributes set with SetAttribute.
     *
     * \param c NodeContainer of the set of nodes on which an CbrApplication
     * will be installed.
     * \returns Container of Ptr to the applications installed.
     */
    ApplicationContainer Install(NodeContainer c) const;

    /**
     * Install an ns3::CbrApplication on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an CbrApplication will be installed.
     * \returns Container of Ptr to the applications installed.
     */
    ApplicationContainer Install(Ptr<Node> node) const;

    /**
     * Install an ns3::CbrApplication on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param nodeName The node on which an CbrApplication will be installed.
     * \returns Container of Ptr to the applications installed.
     */
    ApplicationContainer Install(std::string nodeName) const;

  private:
    /**
     * \internal
     * Install an ns3::CbrApplication on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an CbrApplication will be installed.
     * \returns Ptr to the application installed.
     */
    Ptr<Application> InstallPriv(Ptr<Node> node) const;
    std::string m_protocol;
    Address m_remote;
    ObjectFactory m_factory;
};

} // namespace ns3

#endif /* CBR_HELPER_H */
