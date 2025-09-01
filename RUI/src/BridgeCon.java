package RUI.src;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.LinkedList;
import java.util.Queue;

public class BridgeCon {
    private int port;
    private boolean conStatus;
    private Queue<String> msgs;
    private ServerSocket serverSocket;
    private Socket clientSocket;
    private Thread listenerThread;

    // Setup input and output streams for communication with the client
    private BufferedReader in;
    private PrintWriter out;

    BridgeCon() {
        port = 9090;
        conStatus = false;
        msgs = new LinkedList<String>();
    }

    public void startUp() throws IOException{
        serverSocket = new ServerSocket(port);
        System.out.println("Server is running and waiting for client connection...");

        clientSocket = serverSocket.accept();
        System.out.println("Client connected!");

        in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
        out = new PrintWriter(clientSocket.getOutputStream(), true);
        startListening();
    }

    private void startListening() {
        listenerThread = new Thread(() -> {
            try {
                while (!Thread.currentThread().isInterrupted()) {
                    String msg = in.readLine();
                    System.out.println("Recieved: "+msg+" from port: " + port);
                    msgs.add(msg);
                }
            } catch (Exception e) {
                msgs.add("LOST");
            }
        });
        listenerThread.start();
    }

    public void send(String msg){
        out.println(msg);
        System.out.println("Sent: "+msg+" from port: " + port);
    }

    synchronized public String recieve() {
        String msg =  msgs.poll();
        System.out.println("-------------Recieved: "+msg+" from port: " + port);
        return ""+msg;
    }

    public void close() {
        if (listenerThread != null) listenerThread.interrupt();
        try {
            clientSocket.close();
            serverSocket.close();
        } catch (IOException e) {}
        System.out.println("Closed Connection");
    }

    public boolean getConStatus() {
        return conStatus;
    }

    public void setConStatus(boolean b) {
        conStatus = b;
    }
}
