#include "BridgeSystem.h"
static String mechActions[] = { "Lower", "Raise" };
static String mechStates[] = { "Raised", "Lowered", "Raising", "Lowering" };
static String overrideStates[] = { "Off", "On" };
static String overrideActions[] = { "On", "Off" };
static String lightStates[] = { "Red", "Green", "Yellow" };
static String alarmStates[] = { "On", "Off" };
static String alarmActions[] = { "Off", "On" };
static String gateActions[] = { "Lower", "Raise" };
static String gateStates[] = { "Raised", "Lowered", "Raising", "Lowering" };

BridgeSystem::BridgeSystem()
  : gates("Gates", gateActions, gateStates, 2,4),
    alarms("Alarms", alarmActions, alarmStates, 2),
    trafficLights("TrafficLights", lightStates, 3),
    bridgeLights("BridgeLights", lightStates, 3),
    mechanism("Bridge_Mechanism", mechActions, mechStates, 2,4),
    override("Override", overrideActions, overrideStates, 2),
    ultra0(), ultra1(), pir() {}

void BridgeSystem::execute(const String& cmd) {
  if (cmd.indexOf("/Bridge_Mechanism/Raise") >= 0) mechanism.raiseNet();
  else if (cmd.indexOf("/Bridge_Mechanism/Lower") >= 0) mechanism.lowerNet();
  else if (cmd.indexOf("/Gates/Raise") >= 0) gates.openNet();
  else if (cmd.indexOf("/Gates/Lower") >= 0) gates.closeNet();
  else if (cmd.indexOf("/Alarms/On") >= 0) alarms.activate();
  else if (cmd.indexOf("/Alarms/Off") >= 0) alarms.deactivate();
  else if (cmd.indexOf("/TrafficLights/Red") >= 0) trafficLights.turnRed();
  else if (cmd.indexOf("/TrafficLights/Green") >= 0) trafficLights.turnGreen();
  else if (cmd.indexOf("/TrafficLights/Yellow") >= 0) trafficLights.turnYellow();
  else if (cmd.indexOf("/BridgeLights/Red") >= 0) bridgeLights.turnRed();
  else if (cmd.indexOf("/BridgeLights/Green") >= 0) bridgeLights.turnGreen();
  else if (cmd.indexOf("/BridgeLights/Yellow") >= 0) bridgeLights.turnYellow();
  else if (cmd.indexOf("/Override/On") >= 0) override.on();
  else if (cmd.indexOf("/Override/Off") >= 0) override.off();
}
