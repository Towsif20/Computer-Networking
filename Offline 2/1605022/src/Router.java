//Work needed
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;

public class Router {

    private int routerId;
    private int numberOfInterfaces;
    private ArrayList<IPAddress> interfaceAddresses;//list of IP address of all interfaces of the router
    private ArrayList<RoutingTableEntry> routingTable;//used to implement DVR
    private ArrayList<Integer> neighborRouterIDs;//Contains both "UP" and "DOWN" state routers
    private Boolean state;//true represents "UP" state and false is for "DOWN" state
    private Map<Integer, IPAddress> gatewayIDtoIP;
    public Router() {
        interfaceAddresses = new ArrayList<>();
        routingTable = new ArrayList<>();
        neighborRouterIDs = new ArrayList<>();

        /**
         * 80% Probability that the router is up
         */
        Random random = new Random();
        double p = random.nextDouble();
        if(p < 0.80) state = true;
        else state = false;

        numberOfInterfaces = 0;

    }

    public Router(int routerId, ArrayList<Integer> neighborRouters, ArrayList<IPAddress> interfaceAddresses, Map<Integer, IPAddress> gatewayIDtoIP) {
        this.routerId = routerId;
        this.interfaceAddresses = interfaceAddresses;
        this.neighborRouterIDs = neighborRouters;
        this.gatewayIDtoIP = gatewayIDtoIP;
        routingTable = new ArrayList<>();



        /**
         * 80% Probability that the router is up
         */
        Random random = new Random();
        double p = random.nextDouble();
        if(p < 0.80) state = true;
        else state = false;

        numberOfInterfaces = interfaceAddresses.size();
    }

    @Override
    public String toString() {
        String string = "";
        string += "Router ID: " + routerId + "\n" + "Interfaces: \n";
        for (int i = 0; i < numberOfInterfaces; i++) {
            string += interfaceAddresses.get(i).getString() + "\t";
        }
        string += "\n" + "Neighbors: \n";
        for(int i = 0; i < neighborRouterIDs.size(); i++) {
            string += neighborRouterIDs.get(i) + "\t";
        }
        return string;
    }



    /**
     * Initialize the distance(hop count) for each router.
     * for itself, distance=0; for any connected router with state=true, distance=1; otherwise distance=Constants.INFTY;
     */
    public void initiateRoutingTable() {


        RoutingTableEntry entry = null;
       for(int i=0;i<NetworkLayerServer.routers.size();i++)
       {
           Router router=NetworkLayerServer.routers.get(i);
           if(router.getRouterId()==getRouterId()) {
              entry= new RoutingTableEntry(getRouterId(),0,getRouterId());
           }

           else if(neighborRouterIDs.contains(router.getRouterId())) {

              if (router.getState()) {
                  entry = new RoutingTableEntry(router.getRouterId(), 1, router.getRouterId());
              } else {
                  entry = new RoutingTableEntry(router.getRouterId(), Constants.INFINITY, Constants.NULL);
              }
          }
           else {
              entry = new RoutingTableEntry(router.getRouterId(), Constants.INFINITY, Constants.NULL);

          }

           addRoutingTableEntry(entry);

       }





        
    }

    /**
     * Delete all the routingTableEntry
     */
    public void clearRoutingTable() {

        routingTable.clear();
    }

    /**
     * Update the routing table for this router using the entries of Router neighbor
     * @param neighbor
     */
    public boolean updateRoutingTable(Router neighbor) {
        double neighbor_dist=0.0;
        for(RoutingTableEntry entry:routingTable)
        {
           if(entry.getRouterId()==neighbor.getRouterId())
                 neighbor_dist=entry.getDistance();
        }




        ArrayList<RoutingTableEntry> updatedTable=new ArrayList<>();
        int update_count=0;
        if(neighbor.getRoutingTable().size()==NetworkLayerServer.routers.size() && routingTable.size()==NetworkLayerServer.routers.size()) {
        for(int i=0;i<routingTable.size();i++) {
            RoutingTableEntry entry = routingTable.get(i);
            RoutingTableEntry neighborTableEntry = neighbor.getRoutingTable().get(i);

            if (entry.getRouterId() == getRouterId()) {
                updatedTable.add(new RoutingTableEntry(getRouterId(), 0, getRouterId()));
            } else {
                if (entry.getDistance() > neighbor_dist + neighborTableEntry.getDistance()) {
                    update_count++;
                    updatedTable.add(new RoutingTableEntry(entry.getRouterId(),
                            neighbor_dist + neighborTableEntry.getDistance(), neighbor.getRouterId()));
                } else {
                    updatedTable.add(entry);
                }
            }
        }

        }

        this.routingTable=updatedTable;

        if(update_count!=0) return true;

        return false;
        
    }

    public boolean sfupdateRoutingTable(Router neighbor) {
        double neighbor_dist=0.0;
        for(RoutingTableEntry entry:routingTable)
        {
            if(entry.getRouterId()==neighbor.getRouterId())
                neighbor_dist=entry.getDistance();
        }


        ArrayList<RoutingTableEntry> updatedTable=new ArrayList<>();
        int update_count=0;


        if(neighbor.getRoutingTable().size()==NetworkLayerServer.routers.size() && routingTable.size()==NetworkLayerServer.routers.size()) {
            for (int i = 0; i < routingTable.size(); i++) {
                RoutingTableEntry entry = routingTable.get(i);
                RoutingTableEntry neighborTableEntry = neighbor.getRoutingTable().get(i);

                if (entry.getRouterId() == getRouterId()) {
                    updatedTable.add(new RoutingTableEntry(getRouterId(), 0, getRouterId()));
                } else {
                    if (entry.getGatewayRouterId() == neighbor.getRouterId() ||
                            (entry.getDistance() > neighbor_dist + neighborTableEntry.getDistance() &&
                                    getRouterId() != neighborTableEntry.getRouterId())) {
                        update_count++;
                        updatedTable.add(new RoutingTableEntry(entry.getRouterId(),
                                neighbor_dist + neighborTableEntry.getDistance(), neighbor.getRouterId()));
                    } else {
                        updatedTable.add(entry);
                    }
                }


            }
        }

        this.routingTable=updatedTable;

        if(update_count!=0) return true;

        return false;
    }

    /**
     * If the state was up, down it; if state was down, up it
     */
    public void revertState() {
        state = !state;
        if(state) { initiateRoutingTable(); }
        else { clearRoutingTable(); }
    }

    public int getRouterId() {
        return routerId;
    }

    public void setRouterId(int routerId) {
        this.routerId = routerId;
    }

    public int getNumberOfInterfaces() {
        return numberOfInterfaces;
    }

    public void setNumberOfInterfaces(int numberOfInterfaces) {
        this.numberOfInterfaces = numberOfInterfaces;
    }

    public ArrayList<IPAddress> getInterfaceAddresses() {
        return interfaceAddresses;
    }

    public void setInterfaceAddresses(ArrayList<IPAddress> interfaceAddresses) {
        this.interfaceAddresses = interfaceAddresses;
        numberOfInterfaces = interfaceAddresses.size();
    }

    public ArrayList<RoutingTableEntry> getRoutingTable() {
        return routingTable;
    }

    public void addRoutingTableEntry(RoutingTableEntry entry) {
        this.routingTable.add(entry);
    }

    public ArrayList<Integer> getNeighborRouterIDs() {
        return neighborRouterIDs;
    }

    public void setNeighborRouterIDs(ArrayList<Integer> neighborRouterIDs) { this.neighborRouterIDs = neighborRouterIDs; }

    public Boolean getState() {
        return state;
    }

    public void setState(Boolean state) {
        this.state = state;
    }

    public Map<Integer, IPAddress> getGatewayIDtoIP() { return gatewayIDtoIP; }

    public void printRoutingTable() {
        System.out.println("Router " + routerId);
        System.out.println("DestID Distance Nexthop");
        for (RoutingTableEntry routingTableEntry : routingTable) {
            System.out.println(routingTableEntry.getRouterId() + " " + routingTableEntry.getDistance() + " " + routingTableEntry.getGatewayRouterId());
        }
        System.out.println("-----------------------");
    }
    public String strRoutingTable() {
        String string = "Router" + routerId + "\n";
        string += "DestID Distance Nexthop\n";
        for (RoutingTableEntry routingTableEntry : routingTable) {
            string += routingTableEntry.getRouterId() + " " + routingTableEntry.getDistance() + " " + routingTableEntry.getGatewayRouterId() + "\n";
        }

        string += "-----------------------\n";
        return string;
    }

}
