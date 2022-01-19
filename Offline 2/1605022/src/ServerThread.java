

import javax.sound.midi.Soundbank;
import java.awt.*;
import java.sql.SQLOutput;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Random;

public class ServerThread implements Runnable {

    NetworkUtility networkUtility;
    EndDevice endDevice;
    String routingpath;

    ServerThread(NetworkUtility networkUtility, EndDevice endDevice) {
        this.networkUtility = networkUtility;
        this.endDevice = endDevice;
        System.out.println("Server Ready for client " + NetworkLayerServer.clientCount);
        new Thread(this).start();
    }

    @Override
    public void run() {
        /**
         * Synchronize actions with client.
         */

      /*  for(Router router:NetworkLayerServer.routers)
            System.out.println("shurute buijha lo-"+router.getRoutingTable().size());*/


        networkUtility.write(NetworkLayerServer.endDevices);
        networkUtility.write(NetworkLayerServer.clientCount);



        for (int i = 0; i < 100; i++)
        {
            routingpath="";
            Object object = networkUtility.read();
            String msg = "";

            if(object instanceof Packet) {

                Packet packet = (Packet) object;

                if (packet.getDestinationIP().getString().equalsIgnoreCase(packet.getSourceIP().getString()))
                {
                    msg += "NIJER KACHE NIJEI MESSAGE PATHAISOS,hop count 0";
                    networkUtility.write(msg);
                }
                else {

                    boolean sendingSuccess = deliverPacket(packet);

                    if (sendingSuccess) {

                        msg += "Successfully sent,hop count= " + packet.hopcount;
                        if (packet.getSpecialMessage().equalsIgnoreCase("SHOW_ROUTE")) {

                            msg += " Here is the routing path:\n" + routingpath;

                        }


                        networkUtility.write(msg);
                    } else {
                        networkUtility.write("PACKET DROPPED,SENDING FAILED");
                    }
                }
            }


        }


        
        /*
        Tasks:
        1. Upon receiving a packet and recipient, call deliverPacket(packet)
        2. If the packet contains "SHOW_ROUTE" request, then fetch the required information
                and send back to client
        3. Either send acknowledgement with number of hops or send failure message back to client
        */
    }


    public Boolean deliverPacket(Packet p) {

     /** find s **/

     IPAddress source_IP=p.getSourceIP();

     IPAddress s_IP=new IPAddress(source_IP.getBytes()[0]+"."+source_IP.getBytes()[1]+"."
             +source_IP.getBytes()[2]+"."+"1");

     Router s=NetworkLayerServer.routerMap.get(NetworkLayerServer.interfacetoRouterID.get(s_IP));


     /** find d **/

        IPAddress dest_IP=p.getDestinationIP();

        IPAddress d_IP=new IPAddress(dest_IP.getBytes()[0]+"."+dest_IP.getBytes()[1]+"."
                +dest_IP.getBytes()[2]+"."+"1");

        Router d=NetworkLayerServer.routerMap.get(NetworkLayerServer.interfacetoRouterID.get(d_IP));



        /** IMPLEMENT FORWARDING **/

        Router var=s;
        p.hopcount++;
        routingpath+=var.strRoutingTable();

       // System.out.println(" delbpaket e while er age -"+routingpath.size());
        while(true)
        {

            Router gateway=null;

            for(int i=0;i<var.getRoutingTable().size();i++)
            {
                RoutingTableEntry entry=var.getRoutingTable().get(i);
                if(entry.getRouterId()==d.getRouterId()) {

                    if (entry.getGatewayRouterId() == -1) {
                        return false;

                    } else {
                        gateway = NetworkLayerServer.routerMap.get(entry.getGatewayRouterId());
                        /** applying 3a **/
                        if (!gateway.getState()) {
                            entry.setDistance(Constants.INFINITY);
                            var.getRoutingTable().set(i, entry);
                            RouterStateChanger.islocked = true;
                            //NetworkLayerServer.DVR(var.getRouterId());
                            NetworkLayerServer.simpleDVR(gateway.getRouterId());
                            RouterStateChanger.islocked = false;
                            return false;

                        }

                        /** applying 3b **/
                        for (int j = 0; j < gateway.getRoutingTable().size(); j++) {
                            RoutingTableEntry entry2 = gateway.getRoutingTable().get(j);
                            if (entry2.getRouterId() == var.getRouterId() && entry2.getDistance() == Constants.INFINITY) {
                                entry2.setDistance(1);
                                gateway.getRoutingTable().set(i, entry);
                                RouterStateChanger.islocked = true;
                           //     NetworkLayerServer.DVR(gateway.getRouterId());

                                NetworkLayerServer.simpleDVR(gateway.getRouterId());

                                RouterStateChanger.islocked = false;
                            }

                        }


                        break;
                    }
                }


            }


           if(gateway==null) return false;




            // changing router
            p.hopcount++;
            var=gateway;
            routingpath+=var.strRoutingTable();
           // System.out.println(" delbpaket e while er pore -"+routingpath.size());
            if(var.getRouterId()==d.getRouterId()) break;


        }

        return true;



        /*
        1. Find the router s which has an interface
                such that the interface and source end device have same network address.
        2. Find the router d which has an interface
                such that the interface and destination end device have same network address.
        3. Implement forwarding, i.e., s forwards to its gateway router x considering d as the destination.
                similarly, x forwards to the next gateway router y considering d as the destination,
                and eventually the packet reaches to destination router d.

            3(a) If, while forwarding, any gateway x, found from routingTable of router r is in down state[x.state==FALSE]
                    (i) Drop packet
                    (ii) Update the entry with distance Constants.INFTY
                    (iii) Block NetworkLayerServer.stateChanger.t
                    (iv) Apply DVR starting from router r.
                    (v) Resume NetworkLayerServer.stateChanger.t

            3(b) If, while forwarding, a router x receives the packet from router y,
                    but routingTableEntry shows Constants.INFTY distance from x to y,
                    (i) Update the entry with distance 1
                    (ii) Block NetworkLayerServer.stateChanger.t
                    (iii) Apply DVR starting from router x.
                    (iv) Resume NetworkLayerServer.stateChanger.t

        4. If 3(a) occurs at any stage, packet will be dropped,
            otherwise successfully sent to the destination router
        */

     //   return true;//eita ami disi
    }

    @Override
    public boolean equals(Object obj) {
        return super.equals(obj); //To change body of generated methods, choose Tools | Templates.
    }
}
