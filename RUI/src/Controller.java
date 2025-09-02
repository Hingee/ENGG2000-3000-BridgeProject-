package RUI.src;

import java.io.IOException;
import java.util.Scanner;

public class Controller {
    private static BridgeCon bridgeCon;
    private static BridgeData data;
    private final static String authInfo = "AUTH RUI_123";

    public static void main(String[] args) throws InterruptedException {
        bridgeCon = new BridgeCon();
        data = new BridgeData();

        //MVP
        Scanner scanner = new Scanner(System.in);
        connect();
        while(true) {
            if(!bridgeCon.getConStatus()) {
                System.out.println("Lost connection, attempting to reconnect...");
                bridgeCon.close();
                connect();
                continue;
            }
            bridgeCon.send("OK");
            bridgeCon.recieve();
            
            if(!bridgeCon.getConStatus()) continue;
            System.out.print("Enter msg: ");
            String msgOut = scanner.nextLine();

            bridgeCon.send(msgOut);
            if(msgOut.equals("QUIT")) break;
            System.out.println(bridgeCon.recieve());
        }
        scanner.close();
        bridgeCon.close();
    }

    static void connect() throws InterruptedException {
        while(!bridgeCon.getConStatus()) {
            try {
                bridgeCon.startUp();
            } catch (IOException e) {
                System.out.println("Connection Failed Trying Again");
                Thread.sleep(1000);
            }
        }
        bridgeCon.send("HELO");
        bridgeCon.recieve();
        bridgeCon.send(authInfo);
        bridgeCon.recieve();
    }

    void update() {
        
    }
}
