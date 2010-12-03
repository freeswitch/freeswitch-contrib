package my.gui;

import org.freeswitch.esl.*;

/**
 *
 * @author acosgrove
 */
public class ESLStuff {

    private ESLconnection con;
    private ESLevent evt;

    public ESLStuff() {
        //esl.eslSetLogLevel(7);
    }

    public boolean isConnected() {
            if ( (con != null) && (con.connected() == 1) ) { return true; }
            else { return false; }
    }

    public boolean connect(String host, String port, String pw) {
        con = new ESLconnection(host,port,pw);
        if (isConnected()) {
            con.events("plain", "all");
            con.send("log 7");
        }
        return isConnected();
    }

    public void disconnect() {
        if (isConnected()) {
        con.disconnect();
        con = null;
        }
    }

    public String[] getData() {
        evt = con.recvEvent();
        String[] values = {evt.getBody(), evt.serialize("")};
        return values;
    }

    public void command(String input) {
        con.send("api " + input);

    }

}
