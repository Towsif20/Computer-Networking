import java.util.ArrayList;
import java.util.Random;

//Work needed
public class Client {
    public static void main(String[] args) throws InterruptedException {
        NetworkUtility networkUtility = new NetworkUtility("127.0.0.1", 4444);
        System.out.println("Connected to server");
        /**
         * Tasks
         */

        ArrayList<EndDevice> clients = (ArrayList<EndDevice>) networkUtility.read();

        int size = clients.size();

        EndDevice device = clients.get(size-1);

        Random random = new Random();

        for (int i=0;i<100;i++)
        {
            String message = getRandomString();
            String special = "";

            int r;

            if (size  > 1)
            {
                r = random.nextInt(size-1);
            }
            else
            {
                r = random.nextInt(size);
            }

            EndDevice receiver = clients.get(r);

            System.out.println(device.getIpAddress().toString());
            System.out.println(receiver.getIpAddress().toString());

            if (i == 20)
            {
                special = "SHOW_ROUTE";
            }

            Packet packet = new Packet(message, special,device.getIpAddress(), receiver.getIpAddress());

            networkUtility.write(packet);

            System.out.println("written");

            String received = networkUtility.read().toString();

            System.out.println(received);
        }
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
    }

    private static String getRandomString()
    {
        String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
        StringBuilder builder = new StringBuilder();
        Random rnd = new Random();
        while (builder.length() < 18) { // length of the random string.
            int index = (int) (rnd.nextFloat() * chars.length());
            builder.append(chars.charAt(index));
        }
        return builder.toString();

    }
}
