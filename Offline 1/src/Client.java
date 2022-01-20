import java.net.Socket;

public class Client
{
    public static void main(String[] args) throws Exception
    {
        while (true)
        {
            Socket socket = new Socket("localhost", HTTPServer.PORT);
            Thread client = new ThreadClient(socket);

            client.start();
        }
    }
}
