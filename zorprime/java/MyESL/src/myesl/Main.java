package myesl;

import org.freeswitch.esl.ESLconnection;
import org.freeswitch.esl.ESLevent;
import org.freeswitch.esl.esl;

public class Main {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        
        System.load("/lib/libesljni.so");

        ESLconnection con = new ESLconnection("127.0.0.1","8021","ClueCon");
        ESLevent evt;

        if (con.connected() == 1) { System.out.println("connected");}
        else { System.out.println("nope");}
        con.events("plain", "all");

        while (con.connected() == 1) {

            evt = con.recvEvent();

            System.out.println(evt.serialize(""));

        }

    }

}
