import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class HTTPServer
{
    public static final int PORT = 1234;

    public static String readFileData(File file, int fileLength) throws IOException
    {
        FileInputStream fileIn = null;
        byte[] fileData = new byte[fileLength];

        try {
            fileIn = new FileInputStream(file);
            fileIn.read(fileData);
        } finally {
            if (fileIn != null)
                fileIn.close();
        }

        return String.valueOf(fileData);
    }

    public static void main(String[] args) throws Exception
    {
        ServerSocket serverSocket = new ServerSocket(PORT);

        String root = "root";
        File rootfile = new File(root);

        if (rootfile.exists())
            System.out.println("root exists");

        else
        {
            System.out.println("no root");
            rootfile.mkdir();
            System.out.println("root created");
        }

        while (true)
        {
            Socket socket = serverSocket.accept();

            Thread thread = new ThreadWorker(socket, root);

            thread.start();
        }



    }

}
