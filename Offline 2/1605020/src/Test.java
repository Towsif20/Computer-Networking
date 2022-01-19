import java.util.ArrayList;

public class Test
{
    public static void main(String[] args)
    {
        ABC abc = new ABC(0, 0);
        ArrayList<ABC> list = new ArrayList<>();
        ArrayList<ABC> list2 = new ArrayList<>();

        list.add(abc);
        list2.add(abc);

        ABC t = list.get(0);
        t.baal = 5;
        //t = new ABC(1,1);
        list.get(0).saal = 5;

        System.out.println(list2.get(0).baal + " " + list2.get(0).saal);
    }
}


class ABC
{
    int baal;
    int saal;

    public ABC(int baal, int saal)
    {
        this.baal = baal;
        this.saal = saal;
    }
}