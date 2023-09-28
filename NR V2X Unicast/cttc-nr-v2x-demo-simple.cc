/**
 *
 * ============================================= libraries ===================================================
 *
 */

#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/nr-module.h"
#include "ns3/lte-module.h"
#include "ns3/stats-module.h"
#include "ns3/config-store-module.h"
#include "ns3/log.h"
#include "ue-mac-pscch-tx-output-stats.h"
#include "ue-mac-pssch-tx-output-stats.h"
#include "ue-phy-pscch-rx-output-stats.h"
#include "ue-phy-pssch-rx-output-stats.h"
#include "ue-to-ue-pkt-txrx-output-stats.h"
#include "v2x-kpi.h"
#include "ue-rlc-rx-output-stats.h"
#include "ns3/antenna-module.h"
#include "ns3/flow-monitor-module.h"
#include <random>
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include <iomanip>
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CttcNrV2xDemoSimple");

#define DDOS_RATE "20Mbps"

/**
 *
 * ============================================= methods ===================================================
 *
 */

/**
 *
 * ================================================================================================
 *
 */

void
ReceivePacketVehicles (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address srcAddress;

  // CustomDataTag tag;

  while ((packet = socket->RecvFrom (srcAddress)))
    {
      // NS_LOG_UNCOND(PrintReceivedRoutingPacket(socket, packet, srcAddress));
    }
}

/**
 *
 * ================================================================================================
 *
 */

Ptr<Socket>
SetupRoutingPacketReceive (Ipv4Address addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, 8000);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&ReceivePacketVehicles));

  return sink;
}

/**
 *
 * ================================================================================================
 *
 */

uint32_t rxByteCounter = 0; //!< Global variable to count RX bytes
uint32_t txByteCounter = 0; //!< Global variable to count TX bytes
uint32_t rxPktCounter = 0; //!< Global variable to count RX packets
uint32_t txPktCounter = 0; //!< Global variable to count TX packets

void
ReceivePacket (Ptr<const Packet> packet, const Address &from)
{
  // NS_UNUSED (from);
  rxByteCounter += packet->GetSize ();
  rxPktCounter++;
}

void
TransmitPacket (Ptr<const Packet> packet)
{
  txByteCounter += packet->GetSize ();
  txPktCounter++;
}

/**
 *
 * ================================================================================================
 *
 */

void
NotifySlPscchScheduling (UeMacPscchTxOutputStats *pscchStats,
                         const SlPscchUeMacStatParameters pscchStatsParams)
{
  pscchStats->Save (pscchStatsParams);
}

void
NotifySlPsschScheduling (UeMacPsschTxOutputStats *psschStats,
                         const SlPsschUeMacStatParameters psschStatsParams)
{
  psschStats->Save (psschStatsParams);
}

void
NotifySlPscchRx (UePhyPscchRxOutputStats *pscchStats,
                 const SlRxCtrlPacketTraceParams pscchStatsParams)
{
  pscchStats->Save (pscchStatsParams);
}

void
NotifySlPsschRx (UePhyPsschRxOutputStats *psschStats,
                 const SlRxDataPacketTraceParams psschStatsParams)
{
  psschStats->Save (psschStatsParams);
}

void
UePacketTraceDb (UeToUePktTxRxOutputStats *stats, Ptr<Node> node, const Address &localAddrs,
                 std::string txRx, Ptr<const Packet> p, const Address &srcAddrs,
                 const Address &dstAddrs, const SeqTsSizeHeader &seqTsSizeHeader)
{
  uint32_t nodeId = node->GetId ();
  uint64_t imsi = node->GetDevice (0)->GetObject<NrUeNetDevice> ()->GetImsi ();
  uint32_t seq = seqTsSizeHeader.GetSeq ();
  uint32_t pktSize = p->GetSize () + seqTsSizeHeader.GetSerializedSize ();

  stats->Save (txRx, localAddrs, nodeId, imsi, pktSize, srcAddrs, dstAddrs, seq);
}

void
NotifySlRlcPduRx (UeRlcRxOutputStats *stats, uint64_t imsi, uint16_t rnti, uint16_t txRnti,
                  uint8_t lcid, uint32_t rxPduSize, double delay)
{
  stats->Save (imsi, rnti, txRnti, lcid, rxPduSize, delay);
}

/**
 *
 * ================================================================================================
 *
 */

void
WriteInitPosGnuScript (std::string posFilename)
{
  std::ofstream outFile;
  std::string filename = "gnu-script-" + posFilename;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  std::string pngFileName;
  pngFileName = posFilename.substr (0, posFilename.rfind ("."));
  outFile << "set terminal png" << std::endl;
  outFile << "set output \"" << pngFileName << ".png\"" << std::endl;
  outFile << "set style line 1 lc rgb 'black' ps 2 pt 7" << std::endl;
  outFile << "unset key" << std::endl;
  outFile << "set grid" << std::endl;
  outFile << "plot \"" << posFilename << "\" using 3:4 with points ls 1";
  outFile.close ();
}

/**
 *
 * ================================================================================================
 *
 */

void
PrintUeInitPosToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<NrUeNetDevice> uedev = node->GetDevice (j)->GetObject<NrUeNetDevice> ();
          if (uedev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << node->GetId () << " " << uedev->GetImsi () << " " << pos.x << " " << pos.y
                      << std::endl;
            }
        }
    }

  WriteInitPosGnuScript (filename);
}

/**
 *
 * ================================================================================================
 *
 */
void
WriteGifGnuScript (std::string MobilityFileName, Time simTime, double speed, Ptr<Node> firstUeNode,
                   Ptr<Node> lastUeNode)
{
  std::ofstream outFile;
  std::string fileName = "gif-script-" + MobilityFileName;
  outFile.open (fileName.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << fileName);
      return;
    }
  outFile << "set term gif animate delay 100" << std::endl;
  std::string gifFileName;
  gifFileName = MobilityFileName.substr (0, MobilityFileName.rfind ("."));
  outFile << "set output \"" << gifFileName << ".gif"
          << "\"" << std::endl;
  outFile << "unset key" << std::endl;
  outFile << "set grid" << std::endl;

  Vector firstNodePos = firstUeNode->GetObject<MobilityModel> ()->GetPosition ();
  Vector LastNodePos = lastUeNode->GetObject<MobilityModel> ()->GetPosition ();
  double xRangeLower = firstNodePos.x - 10.0;
  double xRangeUpper = simTime.GetSeconds () * speed + LastNodePos.x;
  double yRangeLower = firstNodePos.y - 10.0;
  double yRangeUpper = LastNodePos.y + 10.0;
  outFile << "set xrange [" << xRangeLower << ":" << xRangeUpper << "]" << std::endl;
  outFile << "set yrange [" << yRangeLower << ":" << yRangeUpper << "]" << std::endl;
  outFile << "do for [i=0:" << simTime.GetSeconds () - 1 << "] {plot \"" << MobilityFileName
          << "\" index i using 4:5}" << std::endl;
}

/**
 *
 * ================================================================================================
 *
 */

void
GetSlBitmapFromString (std::string slBitMapString, std::vector<std::bitset<1>> &slBitMapVector)
{
  static std::unordered_map<std::string, uint8_t> lookupTable = {
      {"0", 0},
      {"1", 1},
  };

  std::stringstream ss (slBitMapString);
  std::string token;
  std::vector<std::string> extracted;

  while (std::getline (ss, token, '|'))
    {
      extracted.push_back (token);
    }

  for (const auto &v : extracted)
    {
      if (lookupTable.find (v) == lookupTable.end ())
        {
          NS_FATAL_ERROR ("Bit type " << v << " not valid. Valid values are: 0 and 1");
        }
      slBitMapVector.push_back (lookupTable[v] & 0x01);
    }
}

/**
 *
 * ================================================================================================
 *
 */

void
SavePositionPerIP (V2xKpi *v2xKpi)
{
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<NrUeNetDevice> uedev = node->GetDevice (j)->GetObject<NrUeNetDevice> ();
          if (uedev)
            {
              Ptr<Ipv4L3Protocol> ipv4Protocol = node->GetObject<Ipv4L3Protocol> ();
              Ipv4InterfaceAddress addresses = ipv4Protocol->GetAddress (1, 0);
              std::ostringstream ueIpv4Addr;
              ueIpv4Addr.str ("");
              ueIpv4Addr << addresses.GetLocal ();
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              v2xKpi->FillPosPerIpMap (ueIpv4Addr.str (), pos);
            }
        }
    }
}

/**
 *
 * ================================================================================================
 *
 */

double
getDistance (NodeContainer vehicles, int rand_attacker, int victim_rand)
{
  double distance;
  Vector coords;
  Vector next_coords;

  Ptr<MobilityModel> mob = vehicles.Get (rand_attacker)->GetObject<MobilityModel> ();
  Ptr<MobilityModel> mob2 = vehicles.Get (victim_rand)->GetObject<MobilityModel> ();
  coords = mob->GetPosition ();
  next_coords = mob2->GetPosition ();
  distance = mob->GetDistanceFrom (mob2);

  return distance;
}

double jitter;
uint32_t sender;
uint32_t receiver;

/**
 *
 * ================================================================================================
 *
 */

static inline std::string
PrintReceivedRoutingPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address dstAddress)
{

  std::ostringstream oss;
  InetSocketAddress receiverId = InetSocketAddress::ConvertFrom (dstAddress);

  Address srcAddress;
  socket->GetSockName (srcAddress);
  InetSocketAddress senderId = InetSocketAddress::ConvertFrom (srcAddress);

  oss << "Simulation time: " << Simulator::Now ().GetSeconds () << " "
      << "Vehicle ID " << senderId.GetIpv4 ();

  if (InetSocketAddress::IsMatchingType (srcAddress))
    {
      oss << " received one packet (" << packet->GetSize () << " bytes) from "
          << receiverId.GetIpv4 ();
    }
  else
    {
      oss << " received one packet!";
    }

  return oss.str ();
}

/**
 *
 * ================================================================================================
 *
 */

Time txStartTime = Seconds (0.0);
Time rxEndTime = Seconds (0.0);

uint64_t pirCounter =
    0; //!< counter to count how many time we computed the PIR. It is used to compute average PIR
Time lastPktRxTime; //!< Global variable to store the RX time of a packet
Time pir; //!< Global varible to store PIR value

void
ComputePir (Ptr<const Packet> packet, const Address &)
{
  if (pirCounter == 0 && lastPktRxTime.GetSeconds () == 0.0)
    {
      // this the first packet, just store the time and get out
      lastPktRxTime = Simulator::Now ();
      return;
    }
  pir = pir + (Simulator::Now () - lastPktRxTime);
  lastPktRxTime = Simulator::Now ();
  pirCounter++;
}

/**
 *
 * ============================================= MAIN ===================================================
 *
 */

int
main (int argc, char *argv[])
{

  /**
   *
   * ============================================= PARAMS ===================================================
   *
   */

  uint16_t interUeDistance = 20; // meters
  bool logging = false;
  bool useIPv6 = false; // default IPV4
  uint32_t udpPacketSizeBe = 200;
  double dataRateBe = 16; // 16 kilobits per second
  Time simTime = Seconds (20);
  Time slBearersActivationTime = Seconds (2.0);
  uint16_t numerologyBwpSl = 2;
  double centralFrequencyBandSl = 5.89e9; // band n47  TDD //Here band is analogous to channel
  uint16_t bandwidthBandSl = 400; // Multiple of 100 KHz; 400 = 40 MHz
  double txPower = 23; // dBm

  // Where we will store the output files.
  std::string simTag = "v2xdemo";
  std::string outputDir = "/home/manee/Downloads/nrRepo/ns-3-dev/scratch/nr-v2x-examples-unicast/";

  std::string tracefile;
  uint16_t  ueNum = 0;
  uint16_t numPairs = 0;

  /**
   *
   * ============================================= CMD INPUT ===================================================
   *
   */

  CommandLine cmd (__FILE__);

  cmd.AddValue ("interUeDistance", "The distance among the UEs in the topology", interUeDistance);
  cmd.AddValue ("logging", "Enable logging", logging);
  cmd.AddValue ("useIPv6", "Use IPv6 instead of IPv4", useIPv6);
  cmd.AddValue ("packetSizeBe", "packet size in bytes to be used by best effort traffic",
                udpPacketSizeBe);
  cmd.AddValue ("dataRateBe", "The data rate in kilobits per second for best effort traffic",
                dataRateBe);
  cmd.AddValue ("simTime", "Simulation time in seconds", simTime);
  cmd.AddValue ("slBearerActivationTime", "Sidelik bearer activation time in seconds",
                slBearersActivationTime);
  cmd.AddValue ("numerologyBwpSl", "The numerology to be used in sidelink bandwidth part",
                numerologyBwpSl);
  cmd.AddValue ("centralFrequencyBandSl",
                "The central frequency to be used for sidelink band/channel",
                centralFrequencyBandSl);
  cmd.AddValue ("bandwidthBandSl", "The system bandwidth to be used for sidelink", bandwidthBandSl);
  cmd.AddValue ("txPower", "total tx power in dBm", txPower);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);
  cmd.AddValue ("outputDir", "directory where to store simulation results", outputDir);
  cmd.AddValue ("tracefile", "path of the ns-3 tracefile", tracefile);
  cmd.AddValue ("numVeh", "Number of vehicles", ueNum);
  cmd.AddValue ("vPairs", "Number of vehicles", numPairs);

  cmd.Parse (argc, argv);
  // Parse the command line
  cmd.Parse (argc, argv);

  // Time finalSlBearersActivationTime = slBearersActivationTime + Seconds (0.01);
  // Time finalSimTime = simTime + finalSlBearersActivationTime;
  // std::cout << "Final Simulation duration " << finalSimTime.GetSeconds () << std::endl;

  NS_ABORT_IF (centralFrequencyBandSl > 6e9);

  /**
   *
   * ============================================= LOGGING TRUE ===================================================
   *
   */

  if (logging)
    {
      LogLevel logLevel =
          (LogLevel) (LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);
      LogComponentEnable ("UdpClient", logLevel);
      LogComponentEnable ("UdpServer", logLevel);
      LogComponentEnable ("LtePdcp", logLevel);
      LogComponentEnable ("NrSlHelper", logLevel);
      LogComponentEnable ("NrSlUeRrc", logLevel);
      LogComponentEnable ("NrUePhy", logLevel);
      LogComponentEnable ("NrSpectrumPhy", logLevel);
    }

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));

  /**
   *
   * ============================================= VEHICLE ASSIGN ===================================================
   *
   */

  NodeContainer ueVoiceContainer;
  std::cout << "VEHICLES COUNT :" << ueNum << std::endl;
  
//numPairs = vehNum / 2;

  if (numPairs == 0)
    {
      numPairs = 5;
    }
  if (ueNum == 0)
    {
      ueNum = 5;
    }
  if (tracefile.empty ())
    {
      ueVoiceContainer.Create (ueNum);

      MobilityHelper mobility;
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      Ptr<ListPositionAllocator> positionAllocUe = CreateObject<ListPositionAllocator> ();
      for (uint16_t i = 0; i < ueNum; i++)
        {
          positionAllocUe->Add (Vector (interUeDistance * i, 0.0, 1.5));
          // std::cout << "interUeDistance: " << interUeDistance * i << std::endl;
        }
      mobility.SetPositionAllocator (positionAllocUe);
      mobility.Install (ueVoiceContainer);
    }
  else
    {
      Ns2MobilityHelper ns2mobility = Ns2MobilityHelper (tracefile);
      ueVoiceContainer.Create (ueNum);
      ns2mobility.Install ();
    }

  /**
   *
   * ============================================= VEHICLE ASSIGN ===================================================
   *
   */


 numPairs=ueNum/2;
  
  std::vector<std::pair<int, int>> randomPairs;
  std::set<int> availableTxIds;
  std::set<int> availableRxIds;
  for (uint16_t i = 0; i < ueNum; ++i)
    {
      availableTxIds.insert (i);
      availableRxIds.insert (i);
    }
  std::random_device rd;
  std::mt19937 gen (rd ());

  while (randomPairs.size () < numPairs)
    {
      std::uniform_int_distribution<int> txDistribution (0, availableTxIds.size () - 1);
      int tx = *std::next (std::begin (availableTxIds), txDistribution (gen));
      std::uniform_int_distribution<int> rxDistribution (0, availableRxIds.size () - 1);
      int rx = *std::next (std::begin (availableRxIds), rxDistribution (gen));
      if (tx != rx)
        {
          availableTxIds.erase (tx);
          availableTxIds.erase (rx);
          availableRxIds.erase (tx);
          availableRxIds.erase (rx);
          randomPairs.push_back (std::make_pair (tx, rx));
        }
    }
  std::cout << "Random Vehicle Pairs (TX, RX):" << std::endl;
  std::cout << "Sequentially Generated Vehicle Pairs (TX, RX):" << std::endl;
  for (uint16_t i = 0; i < numPairs; ++i)
    { 
      std::cout << "Pair "<< i + 1 <<": "<<"(" << randomPairs[i].first << ", " << randomPairs[i].second << ")" << std::endl;
    }
    
     // Find dropped vehicle IDs
    std::vector<int> droppedIds;
    for (int i = 0; i < ueNum; ++i) {
        if (availableTxIds.count(i) > 0 || availableRxIds.count(i) > 0) {
            droppedIds.push_back(i);
        }
    }

    std::cout << "Dropped Vehicle IDs:";
    for (int id : droppedIds) {
        std::cout << " " << id;
    }
    std::cout << std::endl;


  /**
   *
   * ============================================= NR HELP CONFIGS ===================================================
   *
   */

  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  nrHelper->SetEpcHelper (epcHelper);

  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;

  CcBwpCreator::SimpleOperationBandConf bandConfSl (centralFrequencyBandSl, bandwidthBandSl,
                                                    numCcPerBand, BandwidthPartInfo::V2V_Highway);

  OperationBandInfo bandSl = ccBwpCreator.CreateOperationBandContiguousCc (bandConfSl);

  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (MilliSeconds (0)));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));
  nrHelper->InitializeOperationBand (&bandSl);
  allBwps = CcBwpCreator::GetAllBwps ({bandSl});

  // Core latency
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("AntennaElement",
                                   PointerValue (CreateObject<IsotropicAntennaModel> ()));
  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (txPower));
  nrHelper->SetUeMacAttribute ("EnableSensing", BooleanValue (false));
  nrHelper->SetUeMacAttribute ("T1", UintegerValue (2));
  nrHelper->SetUeMacAttribute ("T2", UintegerValue (33));
  nrHelper->SetUeMacAttribute ("ActivePoolId", UintegerValue (0));
  nrHelper->SetUeMacAttribute ("ReservationPeriod", TimeValue (MilliSeconds (100)));
  nrHelper->SetUeMacAttribute ("NumSidelinkProcess", UintegerValue (4));
  nrHelper->SetUeMacAttribute ("EnableBlindReTx", BooleanValue (true));
  nrHelper->SetUeMacAttribute ("SlThresPsschRsrp", IntegerValue (-123));
  uint8_t bwpIdForGbrMcptt = 0;
  nrHelper->SetBwpManagerTypeId (TypeId::LookupByName ("ns3::NrSlBwpManagerUe"));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_MC_PUSH_TO_TALK",
                                               UintegerValue (bwpIdForGbrMcptt));
  std::set<uint8_t> bwpIdContainer;
  bwpIdContainer.insert (bwpIdForGbrMcptt);
  NetDeviceContainer ueVoiceNetDev = nrHelper->InstallUeDevice (ueVoiceContainer, allBwps);

  for (auto it = ueVoiceNetDev.Begin (); it != ueVoiceNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }
  Ptr<NrSlHelper> nrSlHelper = CreateObject<NrSlHelper> ();
  nrSlHelper->SetEpcHelper (epcHelper);
  std::string errorModel = "ns3::NrEesmIrT1";
  nrSlHelper->SetSlErrorModel (errorModel);
  nrSlHelper->SetUeSlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel));

  nrSlHelper->SetNrSlSchedulerTypeId (NrSlUeMacSchedulerSimple::GetTypeId ());
  nrSlHelper->SetUeSlSchedulerAttribute ("FixNrSlMcs", BooleanValue (true));
  nrSlHelper->SetUeSlSchedulerAttribute ("InitialNrSlMcs", UintegerValue (14));

  nrSlHelper->PrepareUeForSidelink (ueVoiceNetDev, bwpIdContainer);

  LteRrcSap::SlResourcePoolNr slResourcePoolNr;

  Ptr<NrSlCommPreconfigResourcePoolFactory> ptrFactory =
      Create<NrSlCommPreconfigResourcePoolFactory> ();

  std::vector<std::bitset<1>> slBitmap = {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1};
  ptrFactory->SetSlTimeResources (slBitmap);
  ptrFactory->SetSlSensingWindow (100); // T0 in ms
  ptrFactory->SetSlSelectionWindow (5);
  ptrFactory->SetSlFreqResourcePscch (10); // PSCCH RBs
  ptrFactory->SetSlSubchannelSize (50);
  ptrFactory->SetSlMaxNumPerReserve (3);
  LteRrcSap::SlResourcePoolNr pool = ptrFactory->CreatePool ();
  slResourcePoolNr = pool;

  LteRrcSap::SlResourcePoolConfigNr slresoPoolConfigNr;
  slresoPoolConfigNr.haveSlResourcePoolConfigNr = true;
  uint16_t poolId = 0;
  LteRrcSap::SlResourcePoolIdNr slResourcePoolIdNr;
  slResourcePoolIdNr.id = poolId;
  slresoPoolConfigNr.slResourcePoolId = slResourcePoolIdNr;
  slresoPoolConfigNr.slResourcePool = slResourcePoolNr;

  LteRrcSap::SlBwpPoolConfigCommonNr slBwpPoolConfigCommonNr;
  slBwpPoolConfigCommonNr.slTxPoolSelectedNormal[slResourcePoolIdNr.id] = slresoPoolConfigNr;

  LteRrcSap::Bwp bwp;
  bwp.numerology = numerologyBwpSl;
  bwp.symbolsPerSlots = 14;
  bwp.rbPerRbg = 1;
  bwp.bandwidth = bandwidthBandSl;

  LteRrcSap::SlBwpGeneric slBwpGeneric;
  slBwpGeneric.bwp = bwp;
  slBwpGeneric.slLengthSymbols = LteRrcSap::GetSlLengthSymbolsEnum (14);
  slBwpGeneric.slStartSymbol = LteRrcSap::GetSlStartSymbolEnum (0);

  LteRrcSap::SlBwpConfigCommonNr slBwpConfigCommonNr;
  slBwpConfigCommonNr.haveSlBwpGeneric = true;
  slBwpConfigCommonNr.slBwpGeneric = slBwpGeneric;
  slBwpConfigCommonNr.haveSlBwpPoolConfigCommonNr = true;
  slBwpConfigCommonNr.slBwpPoolConfigCommonNr = slBwpPoolConfigCommonNr;

  LteRrcSap::SlFreqConfigCommonNr slFreConfigCommonNr;

  for (const auto &it : bwpIdContainer)
    {
      slFreConfigCommonNr.slBwpList[it] = slBwpConfigCommonNr;
    }

  // Configure the TddUlDlConfigCommon IE
  LteRrcSap::TddUlDlConfigCommon tddUlDlConfigCommon;
  tddUlDlConfigCommon.tddPattern = "DL|DL|DL|F|UL|UL|UL|UL|UL|UL|";

  // Configure the SlPreconfigGeneralNr IE
  LteRrcSap::SlPreconfigGeneralNr slPreconfigGeneralNr;
  slPreconfigGeneralNr.slTddConfig = tddUlDlConfigCommon;

  // Configure the SlUeSelectedConfig IE
  LteRrcSap::SlUeSelectedConfig slUeSelectedPreConfig;
  slUeSelectedPreConfig.slProbResourceKeep = 0;
  // Configure the SlPsschTxParameters IE
  LteRrcSap::SlPsschTxParameters psschParams;
  psschParams.slMaxTxTransNumPssch = 5;
  // Configure the SlPsschTxConfigList IE
  LteRrcSap::SlPsschTxConfigList pscchTxConfigList;
  pscchTxConfigList.slPsschTxParameters[0] = psschParams;
  slUeSelectedPreConfig.slPsschTxConfigList = pscchTxConfigList;

  LteRrcSap::SidelinkPreconfigNr slPreConfigNr;
  slPreConfigNr.slPreconfigGeneral = slPreconfigGeneralNr;
  slPreConfigNr.slUeSelectedPreConfig = slUeSelectedPreConfig;
  slPreConfigNr.slPreconfigFreqInfoList[0] = slFreConfigCommonNr;

  nrSlHelper->InstallNrSlPreConfiguration (ueVoiceNetDev, slPreConfigNr);
  int64_t stream = 1;
  stream += nrHelper->AssignStreams (ueVoiceNetDev, stream);
  stream += nrSlHelper->AssignStreams (ueVoiceNetDev, stream);

  /**
   *
   * ============================================= IP ASSIGN ===================================================
   *
   */

  InternetStackHelper internet;
  internet.Install (ueVoiceContainer);
  stream += internet.AssignStreams (ueVoiceContainer, stream);
  uint32_t dstL2Id = 255;
  Ipv4Address groupAddress4 ("255.0.0.0"); // use multicast address as destination
  Address remoteAddress;
  Address localAddress;
  uint16_t port = 8000;
  Ptr<LteSlTft> tft;
  Ipv4InterfaceContainer ueIpIface;
  if (!useIPv6)
    {

      ueIpIface = epcHelper->AssignUeIpv4Address (ueVoiceNetDev);
      remoteAddress = InetSocketAddress (groupAddress4, port);
      localAddress = InetSocketAddress (Ipv4Address::GetAny (), port);
      tft = Create<LteSlTft> (LteSlTft::Direction::BIDIRECTIONAL, LteSlTft::CommType::GroupCast,
                              groupAddress4, dstL2Id);
      nrSlHelper->ActivateNrSlBearer (slBearersActivationTime, ueVoiceNetDev, tft);
    }

  /**
   *
   * ============================================= APPLICATION CONFIG ===================================================
   *
   */

  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetStream (stream);
  startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (0.10));

  double realAppStart = 0.0;
  // double txAppDuration = 0.0;

  // Trace receptions; use the following to be robust to node ID changes
  // std::ostringstream path;

  Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
  int64_t stream2 = 2;
  var->SetStream (stream2);

  /**
   *
   * ============================================= VEHICLE ASSIGN ===================================================
   *
   */

  // for (double time = 3; time < simTime.GetSeconds(); time++)
  //  {
  OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address ());
  std::string dataRateBeString = std::to_string (dataRateBe) + "kb/s";
  onoff1.SetConstantRate (DataRate (dataRateBeString));

  ApplicationContainer app;

  for (uint16_t i = 0; i < numPairs; ++i)
    {
      jitter = (double) rand () / RAND_MAX;

      Ptr<MobilityModel> mobiModltx =
          ueVoiceContainer.Get (randomPairs[i].first)->GetObject<MobilityModel> ();
      Ptr<MobilityModel> mobiModlrx =
          ueVoiceContainer.Get (randomPairs[i].second)->GetObject<MobilityModel> ();

      double distance = mobiModltx->GetDistanceFrom (mobiModlrx);

      Ipv4Address txAdrs = ueVoiceContainer.Get (randomPairs[i].first)
                               ->GetObject<Ipv4L3Protocol> ()
                               ->GetAddress (1, 0)
                               .GetLocal ();
      Ipv4Address rxAdrs = ueVoiceContainer.Get (randomPairs[i].second)
                               ->GetObject<Ipv4L3Protocol> ()
                               ->GetAddress (1, 0)
                               .GetLocal ();
      std::cout << "  (Tx Vehicle Num : " << randomPairs[i].first << " - Tx Address : " << txAdrs
                << ") --> (Rx Vehicle Num : " << randomPairs[i].second << " - Rx Address : " << rxAdrs
                << ") => Distance : " << distance << " meters" << std::endl;

      AddressValue remoteAddress (
          InetSocketAddress (ueIpIface.GetAddress (randomPairs[i].second), port));
      onoff1.SetAttribute ("Remote", remoteAddress);

      app = onoff1.Install (ueVoiceContainer.Get (randomPairs[i].first));
      app.Start (Seconds (3 + i + jitter));
      app.Stop (Seconds (Now ().GetSeconds ()));
    }
  //}

  /**
   *
   * ============================================= DB SETUP ===================================================
   *
   */

  std::string exampleName = simTag + "-" + "nr-v2x-simple-demo";
  SQLiteOutput db (outputDir + exampleName + ".db");

  UeMacPscchTxOutputStats pscchStats;
  pscchStats.SetDb (&db, "pscchTxUeMac");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/"
                                 "ComponentCarrierMapUe/*/NrUeMac/SlPscchScheduling",
                                 MakeBoundCallback (&NotifySlPscchScheduling, &pscchStats));

  UeMacPsschTxOutputStats psschStats;
  psschStats.SetDb (&db, "psschTxUeMac");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/"
                                 "ComponentCarrierMapUe/*/NrUeMac/SlPsschScheduling",
                                 MakeBoundCallback (&NotifySlPsschScheduling, &psschStats));

  UePhyPscchRxOutputStats pscchPhyStats;
  pscchPhyStats.SetDb (&db, "pscchRxUePhy");
  Config::ConnectWithoutContext (
      "/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/"
      "NrSpectrumPhyList/*/RxPscchTraceUe",
      MakeBoundCallback (&NotifySlPscchRx, &pscchPhyStats));

  UePhyPsschRxOutputStats psschPhyStats;
  psschPhyStats.SetDb (&db, "psschRxUePhy");
  Config::ConnectWithoutContext (
      "/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/"
      "NrSpectrumPhyList/*/RxPsschTraceUe",
      MakeBoundCallback (&NotifySlPsschRx, &psschPhyStats));

  UeRlcRxOutputStats ueRlcRxStats;
  ueRlcRxStats.SetDb (&db, "rlcRx");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/"
                                 "ComponentCarrierMapUe/*/NrUeMac/RxRlcPduWithTxRnti",
                                 MakeBoundCallback (&NotifySlRlcPduRx, &ueRlcRxStats));

  UeToUePktTxRxOutputStats pktStats;
  pktStats.SetDb (&db, "pktTxRx");

  Time simStopTime = simTime + slBearersActivationTime + Seconds (realAppStart);

  /**
   *
   * ============================================= FLOW METER ===================================================
   *
   */

  // Flow monitor
  FlowMonitorHelper flowHelper;
  Ptr<ns3::FlowMonitor> monitor = flowHelper.InstallAll ();
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  Simulator::Stop (simStopTime);
  Simulator::Run ();

  monitor->SerializeToXmlFile ("unicastPairsV2xFlow.xml", true, true);
  // AnimationInterface anim ("unicastPairsV2xNetAnim.xml");
  //   Print flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  double averageFlowThroughput = 0.0;
  double averageFlowDelay = 0.0;
  uint64_t totalTxBytes = 0;
  uint64_t totalTxPackets = 0;
  uint64_t totalRxBytes = 0;
  uint64_t totalRxPackets = 0;

  std::ofstream outFile;
  std::string filename = outputDir + "/" + simTag;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  if (!outFile.is_open ())
    {
      std::cerr << "Can't open file " << filename << std::endl;
      return 1;
    }

  outFile.setf (std::ios_base::fixed);

  /**
   *
   * ============================================= flow cal ===================================================
   *
   */

  double flowDuration = (simTime.GetSeconds () - Seconds (var->GetValue (1.0, 2.0)).GetSeconds ());
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin ();
       iter != stats.end (); ++iter)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
      
      std::cout << "Pair: "<< t.sourceAddress << " -> " << t.destinationAddress << std::endl;
      std::cout << "  Tx Packets: " << iter->second.txPackets << std::endl;
      std::cout << "  Tx Bytes:" << iter->second.txBytes << std::endl;
      std::cout << "  Rx Packets: " << iter->second.rxPackets << std::endl;
      std::cout << "  Rx Bytes:" << iter->second.rxBytes << std::endl;
      std::cout << "  Throughput: " << iter->second.rxBytes * 8.0 / flowDuration / 1000.0 << " kbps"
                << std::endl;
      std::cout << "  Delay: " << iter->second.delaySum.GetSeconds () << " seconds" << std::endl;

      if (iter->second.rxPackets > 0)
        {
          // Measure the duration of the flow from receiver's perspective
          averageFlowThroughput += iter->second.rxBytes * 8.0 / flowDuration / 1000 / 1000;
          averageFlowDelay += 1000 * iter->second.delaySum.GetSeconds () / iter->second.rxPackets;

          outFile << "  Throughput: " << iter->second.rxBytes * 8.0 / flowDuration / 1000 / 1000
                  << " Mbps\n";
          outFile << "  Mean delay:  "
                  << 1000 * iter->second.delaySum.GetSeconds () / iter->second.rxPackets << " ms\n";
          //outFile << "  Mean upt:  " << i->second.uptSum / i->second.rxPackets / 1000/1000 << " Mbps \n";
          outFile << "  Mean jitter:  "
                  << 1000 * iter->second.jitterSum.GetSeconds () / iter->second.rxPackets << " ms\n";
        }
    }
  double totalThroughput = 0.0;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();
       i != stats.end (); ++i)
    {

      totalThroughput = averageFlowThroughput;
    }

  std::cout << "\nTotal Throughput: " << totalThroughput << " Mbps" << std::endl;
  std::cout << "Mean flow delay: " << averageFlowDelay / stats.size () << " ms" << std::endl;

  double totalFlowDuration = 0.0;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();
       i != stats.end (); ++i)
    {

      double flowDurSeconds =
          std::abs ((i->second.timeLastRxPacket - i->second.timeFirstTxPacket).GetSeconds ());
      totalFlowDuration += flowDurSeconds;
    }

  double averageFlowDuration = totalFlowDuration / stats.size ();

  std::cout << "Average Flow Duration: " << averageFlowDuration << " s" << std::endl;

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();
       i != stats.end (); ++i)
    {

      totalTxBytes += i->second.txBytes;
      totalTxPackets += i->second.txPackets;
    }

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();
       i != stats.end (); ++i)
    {

      totalRxBytes += i->second.rxBytes;
      totalRxPackets += i->second.rxPackets;
    }

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();
       i != stats.end (); ++i)
    {

      // Update the total counters
      totalTxBytes += i->second.txBytes;
      totalTxPackets += i->second.txPackets;
      totalRxBytes += i->second.rxBytes;
      totalRxPackets += i->second.rxPackets;
    }

  // Calculate average values
  double averageTxBytes = static_cast<double> (totalTxBytes) / stats.size ();
  double averageTxPackets = static_cast<double> (totalTxPackets) / stats.size ();
  double averageRxBytes = static_cast<double> (totalRxBytes) / stats.size ();
  double averageRxPackets = static_cast<double> (totalRxPackets) / stats.size ();

  std::cout << "\nTotal Tx Packets: " << totalTxPackets << " packets" << std::endl;
  std::cout << "Total Tx Bytes: " << totalTxBytes << " bytes" << std::endl;

  // Print total rx bytes and total rx packets of all flows

  std::cout << "Total Rx Packets: " << totalRxPackets << " packets" << std::endl;
  std::cout << "Total Rx Bytes: " << totalRxBytes << " bytes" << std::endl;

  // Print average values
  std::cout << "\nAverage Tx Bytes: " << averageTxBytes << " bytes" << std::endl;
  std::cout << "Average Tx Packets: " << averageTxPackets << " packets" << std::endl;
  std::cout << "Average Rx Bytes: " << averageRxBytes << " bytes" << std::endl;
  std::cout << "Average Rx Packets: " << averageRxPackets << " packets" << std::endl;

    std::cout << "Average  Packets Loss: " << averageTxPackets - averageRxPackets << " packets" << std::endl;

  /*
   * VERY IMPORTANT: Do not forget to empty the database cache, which would
   * dump the data store towards the end of the simulation in to a database.
   */
  pktStats.EmptyCache ();
  pscchStats.EmptyCache ();
  psschStats.EmptyCache ();
  pscchPhyStats.EmptyCache ();
  psschPhyStats.EmptyCache ();
  ueRlcRxStats.EmptyCache ();
  Simulator::Destroy ();

  return 0;
}
