package RUI.src;

import java.io.IOException;
import java.util.Scanner;

public class Controller {
    private static BridgeCon bridgeCon;
    private static BridgeData data;
    private final static String authInfo = "AUTH RUI_123";

    public static void main(String[] args) {
        bridgeCon = new BridgeCon();
        data = new BridgeData();
        

        //MVP
        Scanner scanner = new Scanner(System.in);
        connect();
        while(true) {
            bridgeCon.send("OK");
            bridgeCon.recieve();

            System.out.print("Enter msg: ");
            String msgOut = scanner.nextLine();

            bridgeCon.send(msgOut);
            if(msgOut.equals("QUIT")) break;

            String msgIn = bridgeCon.recieve();
            if(msgIn.equals("LOST")) {
                System.out.println("Lost connection, attempting to reconnect...");
                bridgeCon.close();
                connect();
            }
        }
        scanner.close();
        bridgeCon.close();
    }

    static void connect() {
        try {
            bridgeCon.startUp();
        } catch (IOException e) {
            e.printStackTrace();
        }
        bridgeCon.send("HELO");
        bridgeCon.recieve();
        bridgeCon.send(authInfo);
        bridgeCon.recieve();
    }

    void update() {
        
    }
}
