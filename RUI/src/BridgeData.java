package RUI.src;

public class BridgeData {
    private String BridgeState;
    private String gateState;
    private boolean isAlarmOn;
    private char trafficLights;
    private char boatLights;

    BridgeData() {
        BridgeState = "";
        gateState = "";
        isAlarmOn = false;
        trafficLights = '.';
        boatLights = '.';
    }

    public String getSTAT() {
        String a = "OFF";
        if(isAlarmOn) a = "ON";
        return "STAT "+BridgeState+" "+gateState+" "+a+" "+trafficLights+" "+boatLights;
    }

    public boolean update(String stat) {
        String[] wordsArray = stat.split("\s+");
        if(!wordsArray[0].equals("STAT")) return false;

        BridgeState = wordsArray[1];
        gateState = wordsArray[2];
        if(wordsArray[3].equals("ON")) isAlarmOn = false;
        else isAlarmOn = false;
        trafficLights = wordsArray[4].charAt(0);
        boatLights = wordsArray[5].charAt(0);

        return true;
    }
}
