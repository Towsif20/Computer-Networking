

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;

//Work needed
public class NetworkLayerServer {

   public static int clientCount = 0;
  public static ArrayList<Router> routers = new ArrayList<>();
  public static RouterStateChanger stateChanger = null;
    static Map<IPAddress,Integer> clientInterfaces = new HashMap<>(); //Each map entry represents number of client end devices connected to the interface
  public static Map<IPAddress, EndDevice> endDeviceMap = new HashMap<>();
   public static ArrayList<EndDevice> endDevices = new ArrayList<>();
    static Map<Integer, Integer> deviceIDtoRouterID = new HashMap<>();
    static Map<IPAddress, Integer> interfacetoRouterID = new HashMap<>();
  public static Map<Integer, Router> routerMap = new HashMap<>();

    public static void main(String[] args) {

        //Task: Maintain an active client list

        ServerSocket serverSocket = null;
        try {
            serverSocket = new ServerSocket(4444,0, InetAddress.getByName("localhost"));
        } catch (IOException ex) {
            Logger.getLogger(NetworkLayerServer.class.getName()).log(Level.SEVERE, null, ex);
        }


        System.out.println("Server Ready: " + serverSocket.getInetAddress().getHostAddress());
        System.out.println("Creating router topology");

        readTopology();
        printRouters();

        initRoutingTables(); //Initialize routing tables for all routers





        //DVR(1); //Update routing table using distance vector routing until convergence
        simpleDVR(1);
        stateChanger = new RouterStateChanger();//Starts a new thread which turns on/off routers randomly depending on parameter Constants.LAMBDA

        while(true) {
            try {
                Socket socket = serverSocket.accept();

                System.out.println("Client" + (clientCount + 1) + " attempted to connect");
                EndDevice endDevice = getClientDeviceSetup();
                clientCount++;
                endDevices.add(endDevice);
                endDeviceMap.put(endDevice.getIpAddress(),endDevice);
                new ServerThread(new NetworkUtility(socket), endDevice);
            } catch (IOException ex) {
                Logger.getLogger(NetworkLayerServer.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    public static void initRoutingTables() {
        for (Router router : routers) {
            //if(router.getState())
            router.initiateRoutingTable();
        }
    }

    public static synchronized void DVR(int startingRouterId) {

        ArrayList<Router> sortedRouters=new ArrayList<>();

        // sort the routers so that startingrouter is first
        for(Router router:routers)
        {
            if(router.getRouterId()==startingRouterId)
            {
                sortedRouters.add(0,router);
            }
            else
            {
                sortedRouters.add(router);
            }
        }






      int itr=0;

        while(true)
        {

            itr++;
            boolean convergence=false;

            for(Router r:sortedRouters)
            {
                ArrayList<RoutingTableEntry> T=r.getRoutingTable();

                ArrayList<Router> N=new ArrayList<>();

                //find routers N which are the actibe neighbors of r
                for(int i:r.getNeighborRouterIDs())
                {
                    Router router= routerMap.get(i);
                    if(router.getState())
                        N.add(router);
                }

                for(Router t:N)
                {
                    if(t.sfupdateRoutingTable(r))
                    {
                        convergence=true;

                    }

                }



            }


            if(!convergence) break;

            if(itr==NetworkLayerServer.routers.size()-1) break;


        }




    }

    public static synchronized void simpleDVR(int startingRouterId) {



        ArrayList<Router> sortedRouters=new ArrayList<>();

        // sort the routers so that startingrouter is first
        for(Router router:routers)
        {
            if(router.getRouterId()==startingRouterId)
            {
                sortedRouters.add(0,router);
            }
            else
            {
                sortedRouters.add(router);
            }
        }

        while(true)
        {

            boolean convergence=false;

          for(Router r:sortedRouters)
          {
              ArrayList<RoutingTableEntry> T=r.getRoutingTable();

              ArrayList<Router> N=new ArrayList<>();

              for(int i:r.getNeighborRouterIDs())
              {
                  Router router= routerMap.get(i);
                  if(router.getState())
                      N.add(router);
              }

              for(Router t:N)
              {
                  if(t.getState() && t.getRoutingTable().size()==15) {
                      if (t.updateRoutingTable(r)) {
                          convergence = true;
                      }
                  }
              }
          }

          if(!convergence) break;


        }


    }

    public static EndDevice getClientDeviceSetup() {
        Random random = new Random(System.currentTimeMillis());
        int r = Math.abs(random.nextInt(clientInterfaces.size()));

        System.out.println("Size: " + clientInterfaces.size() + "\n" + r);

        IPAddress ip = null;
        IPAddress gateway = null;

        int i = 0;
        for (Map.Entry<IPAddress, Integer> entry : clientInterfaces.entrySet()) {
            IPAddress key = entry.getKey();
            Integer value = entry.getValue();
            if(i == r) {
                gateway = key;
                ip = new IPAddress(gateway.getBytes()[0] + "." + gateway.getBytes()[1] + "." + gateway.getBytes()[2] + "." + (value+2));
                value++;
                clientInterfaces.put(key, value);
                deviceIDtoRouterID.put(endDevices.size(), interfacetoRouterID.get(key));
                break;
            }
            i++;
        }

        EndDevice device = new EndDevice(ip, gateway, endDevices.size());

        System.out.println("Device : " + ip + "::::" + gateway);
        return device;
    }

    public static void printRouters() {
        for(int i = 0; i < routers.size(); i++) {
            System.out.println("------------------\n" + routers.get(i));
        }
    }

    public static String strrouters() {
        String string = "";
        for (int i = 0; i < routers.size(); i++) {
            string += "\n------------------\n" + routers.get(i).strRoutingTable();
        }
        string += "\n\n";
        return string;
    }

    public static void readTopology() {
        Scanner inputFile = null;
        try {
            inputFile = new Scanner(new File("topology.txt"));
            //skip first 27 lines
            int skipLines = 27;
            for(int i = 0; i < skipLines; i++) {
                inputFile.nextLine();
            }

            //start reading contents
            while(inputFile.hasNext()) {
                inputFile.nextLine();
                int routerId;
                ArrayList<Integer> neighborRouters = new ArrayList<>();
                ArrayList<IPAddress> interfaceAddrs = new ArrayList<>();
                Map<Integer, IPAddress> interfaceIDtoIP = new HashMap<>();

                routerId = inputFile.nextInt();

                int count = inputFile.nextInt();
                for(int i = 0; i < count; i++) {
                    neighborRouters.add(inputFile.nextInt());
                }
                count = inputFile.nextInt();
                inputFile.nextLine();

                for(int i = 0; i < count; i++) {
                    String string = inputFile.nextLine();
                    IPAddress ipAddress = new IPAddress(string);
                    interfaceAddrs.add(ipAddress);
                    interfacetoRouterID.put(ipAddress, routerId);

                    /**
                     * First interface is always client interface
                     */
                    if(i == 0) {
                        //client interface is not connected to any end device yet
                        clientInterfaces.put(ipAddress, 0);
                    }
                    else {
                        interfaceIDtoIP.put(neighborRouters.get(i - 1), ipAddress);
                    }
                }
                Router router = new Router(routerId, neighborRouters, interfaceAddrs, interfaceIDtoIP);
                routers.add(router);
                routerMap.put(routerId, router);
            }


        } catch (FileNotFoundException ex) {
            Logger.getLogger(NetworkLayerServer.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

}
