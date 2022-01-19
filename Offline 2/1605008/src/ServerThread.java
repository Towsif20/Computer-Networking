

import java.util.ArrayList;
import java.util.Map;
import java.util.Random;

public class ServerThread implements Runnable {

    NetworkUtility networkUtility;
    EndDevice endDevice;

    public String routingPath = "Routing Path : \n";
    public ArrayList<Router> routersPassed = new ArrayList<>();

    ServerThread(NetworkUtility networkUtility, EndDevice endDevice) {
        this.networkUtility = networkUtility;
        this.endDevice = endDevice;
        System.out.println("Server Ready for client " + NetworkLayerServer.clientCount);
        NetworkLayerServer.clientCount++;
        new Thread(this).start();
    }

    @Override
    public void run() {
        /**
         * Synchronize actions with client.
         */
        
        /*
        Tasks:
        1. Upon receiving a packet and recipient, call deliverPacket(packet)
        2. If the packet contains "SHOW_ROUTE" request, then fetch the required information
                and send back to client
        3. Either send acknowledgement with number of hops or send failure message back to client
        */

        this.networkUtility.write(NetworkLayerServer.activeClientList);
        //System.out.println(this.endDevice.getIpAddress());

        for(int j=0 ; j<100 ; j++) {
            routingPath = "Routing Path : \n";
            routersPassed = new ArrayList<>();

            Packet packet = (Packet) this.networkUtility.read();
            //System.out.println("go");
            boolean success = deliverPacket(packet);

            if (packet.getSpecialMessage().equals("Show Route")) {
                String response = "\n\n";
                response += routingPath;
                response += "\nHop Count : " + Integer.toString(packet.hopcount) + "\n\n";
                for (int i = 0; i < routersPassed.size(); i++) {
                    response += routersPassed.get(i).strRoutingTable();
                }

                if (success == true) {
                    //String acknowledge = "Successful\n";
                    response += "\n\nSuccess , Hops : " + packet.hopcount;
                } else {
                    response += "\n\nFailed";
                }

                networkUtility.write(response);
            }
            else {

                if (success == true) {
                    //String acknowledge = "Successful\n";
                    networkUtility.write("Success , Hops : " + packet.hopcount);
                } else {
                    networkUtility.write("Failed");
                }
            }
        }

        if(networkUtility.read().equals("Done"))
        {
            for(int i=0 ; i<NetworkLayerServer.activeClientList.size() ; i++)
            {
                if(NetworkLayerServer.activeClientList.get(i).getIpAddress().toString().equals(this.endDevice.getIpAddress().toString()))
                {
                    NetworkLayerServer.activeClientList.remove(i);
                }
            }
        }
    }


    public Boolean deliverPacket(Packet p) {

        
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


        //IPAddress sourceIP = p.getSourceIP();

        /*for(Map.Entry<IPAddress , EndDevice> entry : NetworkLayerServer.endDeviceMap.entrySet())
        {
            IPAddress temp = entry.getKey();
            if(temp.equals(sourceIP))
                System.out.println("why?");
            System.out.println(temp.toString());
            System.out.println(sourceIP.toString());
            //System.out.println(entry.getValue().getIpAddress());
        }*/

        //System.out.println(sourceIP.toString());
        //System.out.println(NetworkLayerServer.endDeviceMap.containsKey(sourceIP));
        //EndDevice sourceEndDevice = NetworkLayerServer.endDeviceMap.get(sourceIP);
        //int sourceEndDeviceID = sourceEndDevice.getDeviceID();
        //int sourceEndDeviceID = NetworkLayerServer.endDeviceMap.get(sourceIP).getDeviceID();
        //int sourceRouterID = NetworkLayerServer.deviceIDtoRouterID.get(sourceEndDeviceID);
        //Router s = NetworkLayerServer.routerMap.get(sourceRouterID);

        IPAddress sourceIP = p.getSourceIP();
        IPAddress destinationIP = p.getDestinationIP();

        Router s = new Router();
        Router d = new Router();

        for(EndDevice endDevice : NetworkLayerServer.endDevices)
        {
            if(sourceIP.toString().equals(endDevice.getIpAddress().toString()))
            {
                //System.out.println("wow");
                s = NetworkLayerServer.routerMap.get(NetworkLayerServer.interfacetoRouterID.get(endDevice.getGateway()));
            }

            if(destinationIP.toString().equals(endDevice.getIpAddress().toString()))
            {
                d = NetworkLayerServer.routerMap.get(NetworkLayerServer.interfacetoRouterID.get(endDevice.getGateway()));
            }
        }
        //System.out.println(s.getRouterId());
        //System.out.println(s.getState());

        /*IPAddress destinationIP = p.getDestinationIP();
        EndDevice destinationEndDevice = NetworkLayerServer.endDeviceMap.get(destinationIP);
        int destinationEndDeviceID = destinationEndDevice.getDeviceID();
        int destinationRouterID = NetworkLayerServer.deviceIDtoRouterID.get(destinationEndDeviceID);
        Router d = NetworkLayerServer.routerMap.get(destinationRouterID);*/

        /*if(s.getState() == false)
        {
            p.hopcount = 1;
            routersPassed.add(s);
            theConnection += Integer.toString(s.getRouterId());

            return false;
        }*/

        //System.out.println(s.getRouterId());
        //System.out.println(d.getRouterId());

        Router now = s;

        if(now.getState() == false) {

            System.out.println("Base Router is down ! ");
            return false;
        }

        while(true){
            /*if(now.getState() == false)
            {
                return false;
            }*/

            p.hopcount++;
            routersPassed.add(now);
            routingPath += Integer.toString(now.getRouterId());
            routingPath += "\n";

            if(now.getRouterId() == d.getRouterId()){
                System.out.println("Received Message : " + p.getMessage());
                return true;
            }

            else
            {
                int nowsTable = now.getRoutingTable().size();
                for(int i=0 ; i<nowsTable ; i++)
                {
                    if(now.getRoutingTable().get(i).getRouterId() == d.getRouterId())
                    {
                        int nextID = now.getRoutingTable().get(i).getGatewayRouterId();

                        if(nextID == -1) {
                            System.out.println("No next Hop !!!");
                            return false;
                        }

                        Router nextHop = NetworkLayerServer.routerMap.get(nextID);

                        if(nextHop.getState() == false)
                        {
                            System.out.println("Packet 'Franced'....I mean dropped !!!");

                            now.getRoutingTable().get(i).setDistance(Constants.INFINITY);
                            now.getRoutingTable().get(i).setGatewayRouterId(-1);

                            RouterStateChanger.islocked = true;
                            NetworkLayerServer.DVR(now.getRouterId());
                            RouterStateChanger.islocked = false;
                            //RouterStateChanger.msg.notify();

                            return false;
                        }

                        int tableSize = nextHop.getRoutingTable().size();
                        for(int j=0 ; j<tableSize ; j++)
                        {
                            if(nextHop.getRoutingTable().get(j).getRouterId() == now.getRouterId())
                            {
                                if(nextHop.getRoutingTable().get(j).getDistance() == Constants.INFINITY) {
                                    nextHop.getRoutingTable().get(j).setDistance(1);
                                    nextHop.getRoutingTable().get(j).setGatewayRouterId(now.getRouterId());

                                    RouterStateChanger.islocked = true;
                                    NetworkLayerServer.DVR(nextHop.getRouterId());
                                    RouterStateChanger.islocked = false;
                                    //RouterStateChanger.msg.notify();
                                }
                                break;
                            }
                        }

                        now = nextHop;

                        break;
                    }
                }
            }
        }

    }

    @Override
    public boolean equals(Object obj) {
        return super.equals(obj); //To change body of generated methods, choose Tools | Templates.
    }
}
