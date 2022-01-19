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
        //state = true;

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
        //state = true;

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

        //System.out.println("go");

        /*if(this.getState() == false)
            return;*/

        int total = NetworkLayerServer.routers.size();

        for(int i=0 ; i<total ; i++)
        {
            RoutingTableEntry routingTableEntry;
            int dis = Constants.INFINITY;
            int gate = -1 ;

            Router router = NetworkLayerServer.routers.get(i);

            if(this.getRouterId() == router.getRouterId())
            {
                dis = 0;
                gate = this.getRouterId();
            }
            else if(this.neighborRouterIDs.contains(router.getRouterId()) && router.getState() == true)
            {
                dis = 1;
                gate = router.getRouterId();
            }

            routingTableEntry = new RoutingTableEntry(router.getRouterId() , dis , gate);
            routingTable.add(routingTableEntry);

        }
    }

    /**
     * Delete all the routingTableEntry
     */
    public void clearRoutingTable() {
        //routingTable.clear();

        for(int i=0 ; i<routingTable.size() ; i++)
        {
            this.routingTable.get(i).setDistance(Constants.INFINITY);
            this.routingTable.get(i).setGatewayRouterId(-1);
        }
    }

    /**
     * Update the routing table for this router using the entries of Router neighbor
     * @param neighbor
     */
    public boolean updateRoutingTable(Router neighbor) {

        boolean forChange = false;
        int total = NetworkLayerServer.routers.size();

        if(neighbor.routingTable.size() == 0)
            //System.out.println("wow");
            return false;

        //System.out.println(total);

        for(int i=0 ; i<total ; i++)
        {

            double tempDistance = neighbor.routingTable.get(i).getDistance() + 1;

            if(this.routingTable.get(i).getDistance() > tempDistance)
            {
                this.routingTable.get(i).setDistance(tempDistance);
                this.routingTable.get(i).setGatewayRouterId(neighbor.getRouterId());

                forChange = true;
            }
            /*else if(this.routingTable.get(i).getDistance() < tempDistance && this.routingTable.get(i).getGatewayRouterId() == neighbor.getRouterId())
            {
                if(this.routingTable.get(i).getDistance() >= Constants.INFINITY && tempDistance >= Constants.INFINITY)
                {
                    continue;
                }
                else if(tempDistance >= Constants.INFINITY)
                    this.routingTable.get(i).setDistance(Constants.INFINITY);
                else
                    this.routingTable.get(i).setDistance(tempDistance);

                forChange = true;
            }*/
        }

        return forChange;
    }

    public boolean sfupdateRoutingTable(Router neighbor) {
        boolean forChange = false;
        int total = NetworkLayerServer.routers.size();

        if(neighbor.routingTable.size() == 0)
            //System.out.println("wow");
            return false;

        for(int i=0 ; i<total ; i++)
        {
            if(neighbor.routingTable.get(i).getGatewayRouterId() == this.getRouterId())
                continue;

            double tempDistance = neighbor.routingTable.get(i).getDistance() + 1;

            if(this.routingTable.get(i).getDistance() > tempDistance)
            {
                this.routingTable.get(i).setDistance(tempDistance);
                this.routingTable.get(i).setGatewayRouterId(neighbor.getRouterId());

                forChange = true;
            }
            else if(this.routingTable.get(i).getDistance() < tempDistance && this.routingTable.get(i).getGatewayRouterId() == neighbor.getRouterId())
            {
                if(this.routingTable.get(i).getDistance() >= Constants.INFINITY && tempDistance >= Constants.INFINITY)
                {
                    continue;
                }
                else if(tempDistance >= Constants.INFINITY)
                    this.routingTable.get(i).setDistance(Constants.INFINITY);
                else
                    this.routingTable.get(i).setDistance(tempDistance);

                forChange = true;
            }
        }

        return forChange;
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
