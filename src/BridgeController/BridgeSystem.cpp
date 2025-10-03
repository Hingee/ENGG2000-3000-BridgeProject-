#include "BridgeSystem.h"

BridgeSystem::BridgeSystem()
    : gateF("Gate Front"), gateB("Gate Back"), alarm0("Alarm 0"), alarm1("Alarm 1"), trafficLights("TrafficLights"), bridgeLights("BridgeLights"), mechanism(), ultra0(), ultra1(), pir(), override() {}

void BridgeSystem::execute(const String& cmd) {
    if (cmd.indexOf("/Bridge_Mechanism/Raise") >= 0) mechanism.raise();
    else if (cmd.indexOf("/Bridge_Mechanism/Lower") >= 0) mechanism.lower();
    else if (cmd.indexOf("/Front Gate/Open") >= 0) gateF.open();
    else if (cmd.indexOf("/Front Gate/Close") >= 0) gateF.close();
    else if (cmd.indexOf("/Back Gate/Open") >= 0) gateB.open();
    else if (cmd.indexOf("/Back Gate/Close") >= 0) gateB.close();
    else if (cmd.indexOf("/Alarm0/On") >= 0) alarm0.activate();
    else if (cmd.indexOf("/Alarm0/Off") >= 0) alarm0.deactivate();
    else if (cmd.indexOf("/Alarm1/On") >= 0) alarm1.activate();
    else if (cmd.indexOf("/Alarm1/Off") >= 0) alarm1.deactivate();
    else if (cmd.indexOf("/TrafficLights/Red") >= 0) trafficLights.turnRed();
    else if (cmd.indexOf("/TrafficLights/Green") >= 0) trafficLights.turnGreen();
    else if (cmd.indexOf("/TrafficLights/Yellow") >= 0) trafficLights.turnYellow();
    else if (cmd.indexOf("/BridgeLights/Red") >= 0) bridgeLights.turnRed();
    else if (cmd.indexOf("/BridgeLights/Green") >= 0) bridgeLights.turnGreen();
    else if (cmd.indexOf("/BridgeLights/Yellow") >= 0) bridgeLights.turnYellow();
    else if (cmd.indexOf("/Override/On") >= 0) override.on();
    else if (cmd.indexOf("/Override/Off") >= 0) override.off();
}
