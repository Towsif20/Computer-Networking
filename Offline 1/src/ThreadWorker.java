import java.io.*;
import java.net.Socket;
import java.nio.file.Files;
import java.util.Date;

public class ThreadWorker extends Thread
{
    public static final int BUFFER_SIZE = 4000;
    Socket socket;
    String root;
    FileOutputStream out;

    public ThreadWorker(Socket socket, String root) throws FileNotFoundException
    {
        this.socket = socket;
        this.root = root;
        out = new FileOutputStream("log.txt", true);
    }

    private void respond(String resp, String type, String content)
    {
        try
        {
            PrintWriter pw = new PrintWriter(socket.getOutputStream());


            pw.write(resp + "\r\n");
            pw.write("Server: Java HTTP Server: 1.1\r\n");
            pw.write("Date: " + new Date() + "\r\n");

            out.write(("\r\n").getBytes());
            out.write((resp + "\r\n").getBytes());
            out.write(("Server: Java HTTP Server: 1.1\r\n").getBytes());
            out.write(("Date: " + new Date() + "\r\n").getBytes());

            if (!resp.toLowerCase().contains("not found"))
            {
                pw.write("Content-Type: " + type + "\r\n");
                pw.write("Content-Length: " + content.length() + "\r\n");

                out.write(("Content-Type: " + type + "\r\n").getBytes());
                out.write(("Content-Length: " + content.length() + "\r\n").getBytes());
            } else if (resp.toLowerCase().contains("not found"))
            {
                //pw.write("Content-Type: " + type + "\r\n");
                //pw.write("Content-Length: 0\r\n");
                pw.write("Content-Length: " + content.length() + "\r\n");
                //out.write(("Content-Length: 0\r\n").getBytes());
            }

            pw.write("\r\n");

            out.write(("\r\n").getBytes());

            if (content != null)
            {
                pw.write(content);
                System.out.println("content : " + content);
            }
            pw.flush();
            pw.close();
        } catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    private void download(File file)
    {

        long size = file.length();
        byte[] buffer = new byte[BUFFER_SIZE];
        try
        {
            PrintWriter pw = new PrintWriter(socket.getOutputStream());

            FileInputStream fis = new FileInputStream(file);

            pw.write("HTTP/1.1 200 OK\r\n");
            pw.write("Server: Java HTTP Server: 1.1\r\n");
            pw.write("Date: " + new Date() + "\r\n");
            String mimeType = Files.probeContentType(file.toPath());
            pw.write("Content-Type: " + mimeType + "\r\n");
            pw.write("Content-Length: " + size + "\r\n");
            pw.write("Content-Transfer-Encoding: binary\r\n");
            pw.write("Content-Disposition: attachment; filename=\"" + file.getName() + "\"\r\n");
            pw.write("\r\n");
            pw.flush();

            out.write(("HTTP/1.1 200 OK\r\n").getBytes());
            out.write(("Server: Java HTTP Server: 1.1\n").getBytes());
            out.write(("Date: " + new Date() + "\r\n").getBytes());
            out.write(("Content-Type: " + mimeType + "\r\n").getBytes());
            out.write(("Content-Length: " + size + "\r\n").getBytes());
            out.write(("Content-Transfer-Encoding: binary\r\n").getBytes());
            out.write(("Content-Disposition: attachment; filename=\"" + file.getName() + "\"\r\n\r\n").getBytes());
            out.write(("\r\n\r\n").getBytes());

            DataOutputStream dos = new DataOutputStream(socket.getOutputStream());

            int len = fis.read(buffer);
            while (len > 0)
            {
                dos.write(buffer, 0, len);
                len = fis.read(buffer);
            }

            dos.flush();
            pw.close();

        } catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    @Override
    public void run()
    {
        BufferedReader br;
        //PrintWriter pw = null;
        String input = null;
        //InputStreamReader in;

        try
        {
            InputStreamReader in = new InputStreamReader(socket.getInputStream());
            br = new BufferedReader(in);
            //pw = new PrintWriter(socket.getOutputStream());
            input = br.readLine();
        } catch (IOException e)
        {
            e.printStackTrace();
        }

        System.out.println("request : " + input);

        if (input != null)
        {
            try
            {
                out.write(input.getBytes());
            } catch (IOException e)
            {
                e.printStackTrace();
            }

            String[] split = input.split(" ");

            String method = split[0];

            String path = split[1];

            System.out.println("path : " + path);

            if (input.length() > 0)
            {
                if (method.startsWith("GET"))
                {
                    path = path.replace("%20", " ");

                    String content = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n" +
                            "\t\t<link rel=\"icon\" href=\"data:,\"></head><br>\n";

                    content += "<body><ul>";

                    int slashCounter = 0;

                    for (int i = 0; i < path.length(); i++)
                    {
                        if (path.charAt(i) == '/')
                            slashCounter++;
                    }

                    if (slashCounter == 1)
                    {
                        if (!path.equals("/") && !path.equals("/root"))
                            root = "";
                        File file = new File(root);

                        System.out.println(root);

                        if (!file.exists())
                        {
                            content = "<html>404 Not found</html>";

                            System.out.println("404 Not found");
                            respond("HTTP/1.1 404 NOT FOUND", null, content);
                            try
                            {
                                socket.close();
                            } catch (IOException e)
                            {
                                e.printStackTrace();
                            }

                            return;
                        }

                        if (file.isDirectory())
                        {
                            for (File f : file.listFiles())
                            {


                                if (f.isDirectory())
                                {
                                    content += "<li><b><a href=\"" + f.getPath() + "\">" + f.getName() + "</a></b></li>";
                                }

                                if (f.isFile())
                                {
                                    content += "<li><a href=\"" + f.getPath() + "\">" + f.getName() + "</a></li>";
                                }
                            }

                            content += "</ul></body></html>";

                            respond("HTTP/1.1 200 OK", "text/html", content);
                        }
                        else if (file.isFile())
                        {
                            download(file);
                        }
                    }
                    else
                    {
                        File file = new File("." + path);

                        if (!file.exists())
                        {
                            content = "<html>404 Not found</html>";
                            System.out.println("404 Not found");
                            respond("HTTP/1.1 404 NOT FOUND", null, content);
                            try
                            {
                                socket.close();
                            } catch (IOException e)
                            {
                                e.printStackTrace();
                            }

                            return;
                        }

                        if (file.isDirectory())
                        {
                            for (File f : file.listFiles())
                            {
                                String link = f.getParentFile().getName() + "/" + f.getName();
                                System.out.println("path " + f.getPath());
                                System.out.println(" link " + link);
                                if (f.isDirectory())
                                {
                                    content += "<li><b><a href=\"" + link + "\">" + f.getName() + "</a></b></li>";
                                }

                                if (f.isFile())
                                {
                                    content += "<li><a href=\"" + link + "\">" + f.getName() + "</a></li>";
                                }
                            }

                            content += "</ul></body></html>";

                            respond("HTTP/1.1 200 OK", "text/html", content);
                        }
                        else if (file.isFile())
                        {
                            download(file);
                        }
                    }
                }
                else if (method.toUpperCase().startsWith("UPLOAD"))
                {
                    System.out.println("input"+input);
                    path = input.replace("UPLOAD ", "");
                    System.out.println("path = " + path);

                    System.out.println(method);
                    System.out.println("path of upload: " + path);

                    File file = new File(root + "\\" + path);

                    int len;
                    byte[] buffer = new byte[BUFFER_SIZE];

                    DataInputStream din = null;
                    FileOutputStream fos = null;

                    try
                    {
                        fos = new FileOutputStream(file);
                        din = new DataInputStream(new BufferedInputStream(socket.getInputStream()));

                    } catch (IOException e)
                    {
                        e.printStackTrace();
                    }

                    try
                    {
                        len = din.read(buffer, 0, BUFFER_SIZE);

                        while (len  > 0)
                        {
                            fos.write(buffer, 0, len);
                            len = din.read(buffer, 0, BUFFER_SIZE);
                        }

                        System.out.println("upload complete");

                        fos.close();
                        din.close();

                    }catch (Exception e)
                    {
                        e.printStackTrace();
                    }

                }

                }


            }
            else
        {
            System.out.println("invalid input");
        }

        try
        {
            out.flush();
            socket.close();
        } catch (IOException e)
        {
            e.printStackTrace();
        }

        }
    }
