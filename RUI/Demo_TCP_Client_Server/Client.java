package RUI.Demo_TCP_Client_Server;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class Client {
    static final String ip = "127.0.0.1";
    static final int destPort = 9090;
        public static void main(String args[]) throws IOException {
        // create a socket to connect to the server running on localhost at port number 9090
        Socket socket = new Socket(ip, destPort);
        
        // Setup output stream to send data to the server
        PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
        
        // Setup input stream to receive data from the server
        BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

        // // Send message to the server
        // out.println("Hello from client!");

        while(true) {
            // Receive response from the server
            String response = in.readLine();
            System.out.println("Server says: " + response);
            if(response.equals("QUIT")) {
                break;
            }
        }

        // Close the socket
        socket.close();
    }
}
