#include "BridgeSystem.h"

BridgeSystem::BridgeSystem()
    : gate(), alarm(), trafficLights("TrafficLights"), bridgeLights("BridgeLights"), mechanism(), ultra0(), ultra1(), pir(), override() {}

void BridgeSystem::execute(const String& cmd) {
    if (cmd.indexOf("/Bridge_Mechanism/Raise") >= 0) mechanism.raise();
    else if (cmd.indexOf("/Bridge_Mechanism/Lower") >= 0) mechanism.lower();
    else if (cmd.indexOf("/Gate/Open") >= 0) gate.open();
    else if (cmd.indexOf("/Gate/Close") >= 0) gate.close();
    else if (cmd.indexOf("/Alarm/On") >= 0) alarm.activate();
    else if (cmd.indexOf("/Alarm/Off") >= 0) alarm.deactivate();
    else if (cmd.indexOf("/TrafficLights/Red") >= 0) trafficLights.turnRed();
    else if (cmd.indexOf("/TrafficLights/Green") >= 0) trafficLights.turnGreen();
    else if (cmd.indexOf("/TrafficLights/Yellow") >= 0) trafficLights.turnYellow();
    else if (cmd.indexOf("/BridgeLights/Red") >= 0) bridgeLights.turnRed();
    else if (cmd.indexOf("/BridgeLights/Green") >= 0) bridgeLights.turnGreen();
    else if (cmd.indexOf("/BridgeLights/Yellow") >= 0) bridgeLights.turnYellow();
    else if (cmd.indexOf("/Override/On") >= 0) override.on();
    else if (cmd.indexOf("/Override/Off") >= 0) override.off();
}
