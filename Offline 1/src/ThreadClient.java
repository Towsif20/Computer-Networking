import java.io.*;
import java.net.Socket;
import java.util.Scanner;

public class ThreadClient extends Thread
{
    private Socket socket;

    File file = null;
    BufferedReader br;
    PrintWriter pw;
    DataOutputStream dos;
    Scanner scanner;
    String input ;

    public ThreadClient(Socket socket) throws Exception
    {
        this.socket = socket;
        scanner = new Scanner(System.in);
        pw = new PrintWriter(socket.getOutputStream());
        dos = new DataOutputStream(socket.getOutputStream());
        br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        input = scanner.nextLine();
        System.out.println(input);
    }

    @Override
    public void run()
    {
        try
        {
            file = new File(input);
            System.out.println(file.getName());

            if (file.exists())
            {
                FileInputStream fis = new FileInputStream(file);
                pw.println("UPLOAD " + file.getName());
                pw.println(file.length());
                pw.println();
                pw.flush();

                byte[] buffer = new byte[ThreadWorker.BUFFER_SIZE];
                int len = fis.read(buffer);

                while (len > 0)
                {
                    dos.write(buffer, 0, len);
                    len = fis.read(buffer);

                }
                System.out.println("upload complete");
                dos.flush();
                fis.close();
            }
            else
            {
                System.out.println("File not found");
            }
        } catch (IOException e)
        {
            e.printStackTrace();
        }

        try
        {
            socket.close();
            br.close();
            pw.close();

        } catch (IOException e)
        {
            e.printStackTrace();
        }


    }
}
