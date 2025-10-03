#include "BridgeSystem.h"

BridgeSystem::BridgeSystem()
    : gateF("Gate Front"), gateB("Gate Back"), alarm0("Alarm 0"), alarm1("Alarm 1"), trafficLights("TrafficLights"), bridgeLights("BridgeLights"), mechanism(), ultra0(), ultra1(), pir(), override() {}

void BridgeSystem::execute(const String& cmd) {
    if (cmd.indexOf("/Bridge_Mechanism/Raise") >= 0) mechanism.raise();
    else if (cmd.indexOf("/Bridge_Mechanism/Lower") >= 0) mechanism.lower();
    else if (cmd.indexOf("/Gates/Open") >= 0) {
        gateF.open();
        gateB.open();
    }else if (cmd.indexOf("/Gates/Close") >= 0) {
        gateF.close();
        gateB.close();
    }else if (cmd.indexOf("/Alarms/On") >= 0) {
        alarm1.activate();
        alarm0.activate();
    }else if (cmd.indexOf("/Alarms/Off") >= 0) {
        alarm1.deactivate();
        alarm0.deactivate();
    }else if (cmd.indexOf("/TrafficLights/Red") >= 0) trafficLights.turnRed();
    else if (cmd.indexOf("/TrafficLights/Green") >= 0) trafficLights.turnGreen();
    else if (cmd.indexOf("/TrafficLights/Yellow") >= 0) trafficLights.turnYellow();
    else if (cmd.indexOf("/BridgeLights/Red") >= 0) bridgeLights.turnRed();
    else if (cmd.indexOf("/BridgeLights/Green") >= 0) bridgeLights.turnGreen();
    else if (cmd.indexOf("/BridgeLights/Yellow") >= 0) bridgeLights.turnYellow();
    else if (cmd.indexOf("/Override/On") >= 0) override.on();
    else if (cmd.indexOf("/Override/Off") >= 0) override.off();
}
