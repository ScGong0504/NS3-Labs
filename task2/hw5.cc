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
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>

using namespace ns3;

/*定义LOG日志*/
NS_LOG_COMPONENT_DEFINE("HW5");  //定义日志模块声明的名字


int main(int argc,char *argv[])
{
LogComponentEnable("HW5",LOG_LEVEL_INFO);  //设置日志的输出等级

/*设置计时单位、定义参数和命令行参数*/
Time::SetResolution(Time::NS);


uint32_t n = 50;  //50个对等节点
float samplingPeriod = 0.05;

CommandLine cmd;

cmd.AddValue("n","Number of wifi and devices",n);
cmd.AddValue("samplingPeriod","Output sampling period in seconds",samplingPeriod);

ConfigStore inputConfig;
inputConfig.ConfigureDefaults();
inputConfig.SetFileFormat(ConfigStore::RAW_TEXT);
inputConfig.SetMode(ConfigStore::LOAD);
inputConfig.SetFilename("input-defaults.txt");

cmd.Parse(argc,argv);
std::string traceFile = "/home/gsc/VanetMobiSim-1.1/jar/ns_trace.txt";


/*创建节点*/
NodeContainer enbNodes; //声明节点容器
NodeContainer ueNodes;
enbNodes.Create(25); //创建
ueNodes.Create(25);


/*创建ltehelper*/
Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

/*移动模型配置*/
Ns2MobilityHelper mobility = Ns2MobilityHelper(traceFile);
mobility.Install();


/*安装网卡设备*/
//给节点配置网卡--LTE-V协议
NetDeviceContainer enbDevices;  //声明一个网络设备Container
NetDeviceContainer ueDevices;
enbDevices = lteHelper->InstallEnbDevice(enbNodes);
ueDevices = lteHelper->InstallUeDevice(ueNodes);


lteHelper->Attach(ueDevices,enbDevices.Get(0));

enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
EpsBearer bearer (q);
lteHelper->ActivateDataRadioBearer (ueDevices, bearer);



/*配置所有仿真场景*/
lteHelper->EnablePhyTraces ();
lteHelper->EnableMacTraces ();
lteHelper->EnableRlcTraces ();
lteHelper->EnablePdcpTraces ();


/*开始和结束仿真*/
Simulator::Stop(Seconds(60.0));
AnimationInterface anim("hw5.xml");
anim.SetMaxPktsPerTraceFile(9999999999);

Simulator::Run();
Simulator::Destroy();

return 0;
}

