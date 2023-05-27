/* 引入头函数和命名空间 */
#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/internet-module.h"
#include "ns3/wave-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

/*定义LOG日志*/
NS_LOG_COMPONENT_DEFINE("HW2");  //定义日志模块声明的名字


/*设置统计数据的函数*/
static void GoodputSampling(std::string fileName,ApplicationContainer app,Ptr<OutputStreamWrapper>stream,float period,float lastnum)
{
  float getpacketnum;
  float losspacketnum = 0;
  float lossRate;
  uint32_t totalPackets = DynamicCast<PacketSink>(app.Get(0))->GetTotalRx();
  getpacketnum = totalPackets/150.0;
  Simulator::Schedule(Seconds(period),&GoodputSampling,fileName,app,stream,period,getpacketnum);
  getpacketnum = totalPackets/150.0-lastnum;
  if(getpacketnum>62) losspacketnum = getpacketnum-62;
  lossRate = losspacketnum/getpacketnum*100;
  *stream->GetStream()<<Simulator::Now().GetSeconds()<<" "<<totalPackets/150<<" "<<getpacketnum<<" "<<lossRate<<"%"<<std::endl;
}

int main(int argc,char *argv[])
{
LogComponentEnable("HW2",LOG_LEVEL_INFO);  //设置日志的输出等级

/*设置计时单位、定义参数和命令行参数*/
Time::SetResolution(Time::NS);


uint32_t nAdHoc = 50;  //50个对等节点
float samplingPeriod = 0.05;

CommandLine cmd;

cmd.AddValue("nAdHoc","Number of wifi and devices",nAdHoc);
cmd.AddValue("samplingPeriod","Output sampling period in seconds",samplingPeriod);
cmd.Parse(argc,argv);

/*创建节点*/
NodeContainer AdHocNode; //声明节点容器
AdHocNode.Create(nAdHoc); //创建


/*安装网卡设备*/
//wifi拓扑
//物理层和信道
YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default(); //物理层
YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default(); //创建信道
wifiPhy.SetChannel(wifiChannel.Create()); //连接物理层和传输通道
wifiPhy.Set("TxGain",DoubleValue(11.3)); //增加传输增益
//MAC层，选择的是AdHocWifiMac
WifiMacHelper mac = WifiMacHelper();
mac.SetType("ns3::AdhocWifiMac","Slot",StringValue("100ms"));
//继续配置wifi
WifiHelper wifi;
wifi.SetStandard(WIFI_PHY_STANDARD_80211a);
wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager","DataMode",StringValue("OfdmRate6Mbps"));
//告诉拓扑使用的速率控制算法的类型

//给节点配置网卡
NetDeviceContainer AdHocDevices;  //声明一个网络设备Container
AdHocDevices = wifi.Install(wifiPhy,mac,AdHocNode);


/*创建网络协议栈*/
/*路由协议*/
NS_LOG_INFO("Enabling AODV routing on all adHoc nodes");
AodvHelper aodv;

InternetStackHelper Internet; //添加栈
Internet.SetRoutingHelper (aodv); //应用AODV协议
Internet.Install(AdHocNode);

//IPv4地址分配
Ipv4AddressHelper address;
address.SetBase("195.1.1.0","255.255.255.0");

Ipv4InterfaceContainer AdHocIp;
AdHocIp = address.Assign(AdHocDevices);


/*移动场景*/
MobilityHelper mobility;

//位置模型
mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
"X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=1000]"),
"Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=1000]"));
//运动模型--固定不动
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

mobility.Install (AdHocNode);


/*安装应用层协议——设置应用程序发送端*/
NS_LOG_INFO("Create Applications.");
uint16_t port = 9999;
OnOffHelper onOff1("ns3::UdpSocketFactory",Address(InetSocketAddress(AdHocIp.GetAddress(0),port)));
onOff1.SetAttribute("OnTime",StringValue("ns3::ConstantRandomVariable[Constant=1]"));
onOff1.SetAttribute("OffTime",StringValue("ns3::ConstantRandomVariable[Constant=0]"));
onOff1.SetAttribute("PacketSize",UintegerValue(150));
onOff1.SetAttribute("DataRate",DataRateValue(DataRate("200kb/s")));

ApplicationContainer apps1 = onOff1.Install(AdHocNode);
apps1.Start(Seconds(50.0));
apps1.Stop(Seconds(52.0));

/*安装应用层协议--设置应用程序接收端*/
PacketSinkHelper sinkHelper("ns3::UdpSocketFactory",Address(InetSocketAddress(Ipv4Address::GetAny(),port)));
ApplicationContainer apps2 = sinkHelper.Install(AdHocNode.Get(0));

apps2.Start(Seconds(0.0));
apps2.Stop(Seconds(52.0));


/*开始和结束仿真*/
Simulator::Stop(Seconds(60.0));
AnimationInterface anim("hw2.xml");
anim.SetMaxPktsPerTraceFile(9999999999);

/*调用函数统计数据*/
AsciiTraceHelper ascii;
Ptr<OutputStreamWrapper> uploadGoodputStream = ascii.CreateFileStream("output.txt");
Simulator::Schedule(Seconds(50.0),&GoodputSampling,"output.txt",apps2,uploadGoodputStream,samplingPeriod,0.0);

Simulator::Run();
Simulator::Destroy();

return 0;
}

