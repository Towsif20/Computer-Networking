import java.util.ArrayList;
import java.util.Random;
import java.util.Scanner;

//Work needed
public class Client {
    public static void main(String[] args) throws InterruptedException {
        NetworkUtility networkUtility = new NetworkUtility("127.0.0.1", 4444);
        System.out.println("Connected to server");
        /**
         * Tasks
         */
        
        /*
        1. Receive EndDevice configuration from server
        2. Receive active client list from server
        3. for(int i=0;i<100;i++)
        4. {
        5.      Generate a random message
        6.      Assign a random receiver from active client list
        7.      if(i==20)
        8.      {
        9.            Send the message and recipient IP address to server and a special request "SHOW_ROUTE"
        10.           Display routing path, hop count and routing table of each router [You need to receive
                            all the required info from the server in response to "SHOW_ROUTE" request]
        11.     }
        12.     else
        13.     {
        14.           Simply send the message and recipient IP address to server.
        15.     }
        16.     If server can successfully send the message, client will get an acknowledgement along with hop count
                    Otherwise, client will get a failure message [dropped packet]
        17. }
        18. Report average number of hops and drop rate
        */

        //EndDevice endDevice = (EndDevice) networkUtility.read();
        //System.out.println(endDevice.getIpAddress());

        ArrayList<EndDevice> availableClients = (ArrayList<EndDevice>) networkUtility.read();
        //availableClients = NetworkLayerServer.activeClientList;
        int activeClients = availableClients.size();
        EndDevice endDevice = availableClients.get(activeClients - 1);

        //System.out.println(endDevice.getIpAddress().toString());

        System.out.println("----------------------" + availableClients.size() + "--------------------------");

        for(int i=0 ; i<100 ; i++)
        {
            String message = messageGenerator();

            Random random = new Random();

            int r;

            if(availableClients.size() > 1) {
                r = random.nextInt(availableClients.size() -1);
            }
            else{
                r = random.nextInt(availableClients.size());
            }

            EndDevice receiver = availableClients.get(r);

            System.out.println(endDevice.getIpAddress().toString());
            System.out.println(receiver.getIpAddress().toString());

            if(i == 20)
            {
                String specialMessage = "Show Route";

                Packet packet = new Packet(message , specialMessage , endDevice.getIpAddress() , receiver.getIpAddress());
                networkUtility.write(packet);

                //System.out.println(networkUtility.read());
            }

            else
            {
                String specialMessage = "";

                Packet packet = new Packet(message , specialMessage , endDevice.getIpAddress() , receiver.getIpAddress());
                networkUtility.write(packet);
            }

            System.out.println(networkUtility.read());

        }

        System.out.println("Press any key to exit : ");

        Scanner scanner = new Scanner(System.in);
        scanner.next();

        networkUtility.write("Done");


    }

    public static String messageGenerator()
    {
        ArrayList<String> messageContainer = new ArrayList<>();
        messageContainer.add("Boycott France");
        messageContainer.add("Macron is completely sick");
        messageContainer.add("Macron needs mental treatment");
        messageContainer.add("Whole France is mentally sick");
        messageContainer.add("Send Macron to Pabna immediately");
        messageContainer.add("Actually Macron is beyond treatment");
        messageContainer.add("Macron.equals(CowDung) will return true");
        messageContainer.add("Public toilets should be named after Macron");
        messageContainer.add("Macron , an idiot");
        messageContainer.add("French people behaves like joker");

        Random random = new Random();
        int r = random.nextInt(messageContainer.size());

        return  messageContainer.get(r);
    }
}
