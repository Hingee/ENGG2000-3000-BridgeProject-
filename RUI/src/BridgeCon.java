package RUI.src;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

public class BridgeCon {
    private boolean conStatus;
    private Queue<String> msgs;
    private Socket socket;
    private Thread listenerThread;
    private final String ip = "192.168.1.1";
    private final int serverPort = 9090;

    // Setup input and output streams for communication with the client
    private BufferedReader in;
    private PrintWriter out;

    BridgeCon() {
        conStatus = false;
        msgs = new ConcurrentLinkedQueue<String>();
    }

    public void startUp() throws IOException{
        socket = new Socket(ip, serverPort);
        System.out.println("Client is connecting to Bridge");

        in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        out = new PrintWriter(socket.getOutputStream(), true);
        conStatus = true;
        startListening();
    }

    private void startListening() {
        listenerThread = new Thread(() -> {
            try {
                while (!Thread.currentThread().isInterrupted()) {
                    String msg = in.readLine();
                    if (msg == null) {
                        // Server closed the connection gracefully
                        setConStatus(false);
                    }else {
                        msgs.add(msg);
                        setConStatus(true);
                    }
                }
            } catch (IOException e) {
                System.out.println("Connection lost: " + e.getMessage());
                setConStatus(false);
            }
        });
        listenerThread.start();
    }

    public boolean send(String msg){
        if(!getConStatus()) return false;
        out.println(msg);
        System.out.println("Sent: "+msg+" from port: " + serverPort);
        return true;
    }

    public String recieve() {
        if(!socket.isConnected() ||!getConStatus()){
            setConStatus(false);
            return "_";
        } 
        String msg = null;
        while(msg == null) {
            msg =  msgs.poll();
            if(msgs.isEmpty()) return ".";
        }
        System.out.println("-------------Recieved: "+msg+" from port: " + serverPort);
        return ""+msg;
    }

    public void close() {
        if (listenerThread != null) listenerThread.interrupt();
        try {
            socket.close();
        } catch (IOException e) {}
        System.out.println("Closed Connection");
    }

    synchronized public boolean getConStatus() {
        return conStatus;
    }

    synchronized public void setConStatus(boolean b) {
        conStatus = b;
    }
}
