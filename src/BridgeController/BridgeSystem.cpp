#include "BridgeSystem.h"
static String mechActions[] = { "Lower", "Raise" };
static String mechStates[] = { "Raised", "Lowered", "Raising", "Lowering" };
static String overrideStates[] = { "Off", "On" };
static String overrideActions[] = { "On", "Off" };
static String boatLightStates[] = { "Red", "Green", "Yellow" };
static String pedLightStates[] = { "Red", "Green"};
static String alarmStates[] = { "On", "Off" };
static String alarmActions[] = { "Off", "On" };
static String gateActions[] = { "Lower", "Raise" };
static String gateStates[] = { "Raised", "Lowered", "Raising", "Lowering" };

BridgeSystem::BridgeSystem()
  : gates("Gates", gateActions, gateStates, 2,4),
    alarms("Alarms", alarmActions, alarmStates, 2),
    pedestrianLights("PedestrianLights", pedLightStates, 2),
    boatLights("BoatLights", boatLightStates, 3),
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
  else if (cmd.indexOf("/PedestrianLights/Red") >= 0) pedestrianLights.turnRed();
  else if (cmd.indexOf("/PedestrianLights/Green") >= 0) pedestrianLights.turnGreen();
  else if (cmd.indexOf("/BoatLights/Red") >= 0) boatLights.turnRed();
  else if (cmd.indexOf("/BoatLights/Green") >= 0) boatLights.turnGreen();
  else if (cmd.indexOf("/BoatLights/Yellow") >= 0) boatLights.turnYellow();
  else if (cmd.indexOf("/Override/On") >= 0) override.on();
  else if (cmd.indexOf("/Override/Off") >= 0) override.off();
}
