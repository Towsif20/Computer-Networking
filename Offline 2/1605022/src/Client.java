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

        //Recieve my end device config

        EndDevice me;





        Object object=networkUtility.read();

        ArrayList<EndDevice> endDevices=(ArrayList) object;
        Object object2=networkUtility.read();
        int clientcount=(Integer)object2;

        me=endDevices.get(clientcount-1);



        for(int i=0;i<100;i++)
        {

            Random random=new Random();
            EndDevice destination= endDevices.get(random.nextInt(endDevices.size()));


            Packet packet=new Packet("Hi I am Jainta ","I love you",me.getIpAddress(), destination.getIpAddress());


            if(i==20)
            {
                packet.setSpecialMessage("SHOW_ROUTE");
            }

           // if(!packet.getDestinationIP().getString().equalsIgnoreCase(packet.getSourceIP().getString()))
            networkUtility.write( (Object) packet );

            Object receivedObj= networkUtility.read();



            System.out.println(receivedObj.toString());

        }
    }
}
