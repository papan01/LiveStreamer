package edu.papan01.livestreamer;

/**
 * Created by ATD on 2018/3/9.
 */

public class Live555Native implements Runnable{

    private long NativeInstance = 0;

    static{
        System.loadLibrary("LiveStreamer");
    }

    private Thread thread = null;

    public Live555Native(){
        thread = new Thread(this);
        thread.setName("Native-Thread");
    }

    public native boolean initialize(int fps,int port);
    private native void loopNative();
    public native void stopNative();
    public native void feedH264Data(byte[] data);
    public native void destroy();


    public void run() {
        loopNative();
    }

    public void doLoop(){
        thread.start();
    }

    public void stop(){
        stopNative();
    }
}
