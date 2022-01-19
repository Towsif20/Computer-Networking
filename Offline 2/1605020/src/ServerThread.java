

import java.util.ArrayList;
import java.util.Random;

public class ServerThread implements Runnable {

    NetworkUtility networkUtility;
    EndDevice endDevice;

    String routingPath;
    ArrayList<Router> routersPassed;

    ServerThread(NetworkUtility networkUtility, EndDevice endDevice) {
        this.networkUtility = networkUtility;
        this.endDevice = endDevice;

        routingPath = "Routing Path +\n";
        routersPassed = new ArrayList<>();
        System.out.println("Server Ready for client " + NetworkLayerServer.clientCount);
        NetworkLayerServer.clientCount++;
        new Thread(this).start();
    }

    @Override
    public void run() {
        /**
         * Synchronize actions with client.
         */

        networkUtility.write(NetworkLayerServer.endDevices);

        for (int i=0;i<100;i++)
        {
            Packet packet = (Packet) networkUtility.read();

            boolean result = deliverPacket(packet);

            if (packet.getSpecialMessage().equalsIgnoreCase("SHOW_ROUTE"))
            {
                String response = "\n" + routingPath + "\nHop Count : " + packet.hopcount + "\n\n";

                for (Router r : routersPassed)
                {
                    response += r.strRoutingTable();
                }

                if (result)
                {
                    //String acknowledge = "Successful\n";
                    response += "\n\nSuccess , Hops : " + packet.hopcount;
                }
                else
                {
                    response += "\n\nFailed";
                }

                networkUtility.write(response);
            }

            else
            {
                if (result)
                {
                    //String acknowledge = "Successful\n";
                    networkUtility.write("Success , Hops : " + packet.hopcount);
                }
                else
                {
                    networkUtility.write("Failed");
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


    public Boolean deliverPacket(Packet p)
    {

        IPAddress source = p.getSourceIP();
        IPAddress destinationIP = p.getDestinationIP();

        Router s = new Router();
        Router d = new Router();

        for (EndDevice device: NetworkLayerServer.endDevices)
        {
            if (source.toString().equals(device.getIpAddress().toString()))
            {
                int id = NetworkLayerServer.interfacetoRouterID.get(device.getGateway());
                s = NetworkLayerServer.routerMap.get(id);
            }

            if (destinationIP.toString().equals(device.getIpAddress().toString()))
            {
                int id = NetworkLayerServer.interfacetoRouterID.get(device.getGateway());
                d = NetworkLayerServer.routerMap.get(id);
            }

        }

        Router current = s;

        if (!current.getState())
        {
            return false;
        }

        while (true)
        {
            p.hopcount++;
            routersPassed.add(current);
            routingPath += current.getRouterId() + "\n";

            if (d.getRouterId() == current.getRouterId())
            {
                return true;
            }

            ArrayList<RoutingTableEntry> currentTable = current.getRoutingTable();

            for (RoutingTableEntry entry: currentTable)
            {
                if (entry.getRouterId() == d.getRouterId())
                {
                    int nextID = entry.getGatewayRouterId();
                    if (nextID == -1)
                        return false;

                    Router next = NetworkLayerServer.routerMap.get(nextID);

                    if (!next.getState())
                    {
                        System.out.println("Packet dropped");
                        entry.setDistance(Constants.INFINITY);
                        entry.setGatewayRouterId(-1);

                        NetworkLayerServer.simpleDVR(current.getRouterId());
                        //NetworkLayerServer.DVR(current.getRouterId());

                        return false;
                    }

                    ArrayList<RoutingTableEntry> nextTable = next.getRoutingTable();

                    for (RoutingTableEntry nextEntry : nextTable)
                    {
                        if (entry.getRouterId() == nextEntry.getRouterId())
                        {
                            if (nextEntry.getDistance() == Constants.INFINITY)
                            {
                                nextEntry.setDistance(1);
                                NetworkLayerServer.simpleDVR(next.getRouterId());

                                break;
                            }
                        }
                    }

                    current = next;

                    break;
                }

            }

        }

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
    }

    @Override
    public boolean equals(Object obj) {
        return super.equals(obj); //To change body of generated methods, choose Tools | Templates.
    }
}
