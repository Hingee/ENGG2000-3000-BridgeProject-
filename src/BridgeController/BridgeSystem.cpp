#include "BridgeSystem.h"

BridgeSystem::BridgeSystem()
    : gates("Gates"), alarms("Alarms"), trafficLights("TrafficLights"), bridgeLights("BridgeLights"), mechanism(), ultra0(), ultra1(), pir(), override() {}

void BridgeSystem::execute(const String& cmd) {
    if (cmd.indexOf("/Bridge_Mechanism/Raise") >= 0) mechanism.signalAction(1);
    else if (cmd.indexOf("/Bridge_Mechanism/Lower") >= 0) mechanism.signalAction(0);
    else if (cmd.indexOf("/Gates/Raise") >= 0) gates.signalAction(1);
    else if (cmd.indexOf("/Gates/Lower") >= 0) gates.signalAction(0);
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
