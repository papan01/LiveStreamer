package edu.papan01.livestreamer;

import android.annotation.TargetApi;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;
import android.os.Build;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Surface;
import android.view.WindowManager;

import java.io.IOException;


public class ScreenService extends Service implements EncoderThread.EncoderListener{

    private static final String DISPLAY_NAME = "LiveStreamer";
    private MediaProjection projection;
    private VirtualDisplay display;

    private MediaCodec encoder;

    private Live555Native liveNative;

    private static final int DFRAME_RATE = 15;
    private static final int RTSP_PORT = 8554;
    private static final int BIT_RATE = 1024;
    private static final int I_FRAME = 2;
    private static final int VIDEO_WIDTH = 480;
    private static final int VIDEO_HEIGHT = 640;
    private int dpi;

    public static final int STATE_STARTED = 0x00;
    public static final int STATE_STOPED = 0x01;

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        DisplayMetrics displayMetrics = new DisplayMetrics();
        WindowManager wm = (WindowManager) this.getSystemService(WINDOW_SERVICE);
        wm.getDefaultDisplay().getRealMetrics(displayMetrics);
        dpi = displayMetrics.densityDpi;

        if (null != intent) {
            if (intent.getAction() != null && intent.getAction().equals("stop")) {
                if (encoder != null) {
                    projection.stop();
                    encoder.stop();
                    encoder.release();
                    display.release();
                    liveNative.stop();
                    liveNative.destroy();
                    stopService(new Intent(this, ScreenService.class));
                    notifyStatusChange(STATE_STOPED);
                }
            } else {
                try {
                    liveNative = new Live555Native();
                    if (liveNative.initialize(DFRAME_RATE,RTSP_PORT)) {
                        liveNative.doLoop();
                        createRecordSession(intent.getIntExtra("resultCode", 0),
                                (Intent) intent.getParcelableExtra("data"));
                        notifyStatusChange(STATE_STARTED);
                    }
                    else
                        {
                        Log.e("ScreenService", "create transport error!");
                    }

                } catch (Exception ex) {
                    ex.printStackTrace();
                    Log.e("ScreenService", "create transport error!");
                }
            }
        }
        return START_STICKY;
    }

    private void notifyStatusChange(int status) {
        Intent mIntent = new Intent();
        mIntent.putExtra("status",status);
        LocalBroadcastManager.getInstance(this).sendBroadcast(mIntent);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private void createRecordSession(int resultCode, Intent data) throws IOException {
        MediaFormat mMediaFormat = MediaFormat.createVideoFormat("video/avc", VIDEO_WIDTH,
                VIDEO_HEIGHT);

        mMediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, BIT_RATE * 1024);
        mMediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, DFRAME_RATE);

        mMediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT,
                MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        mMediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, I_FRAME);

        encoder = MediaCodec.createEncoderByType("video/avc");

        encoder.configure(mMediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

        Surface surface = encoder.createInputSurface();

        MediaProjectionManager projectionManager = (MediaProjectionManager) getSystemService(Context.MEDIA_PROJECTION_SERVICE);
        projection = projectionManager.getMediaProjection(resultCode, data);
        display = projection.createVirtualDisplay(DISPLAY_NAME, VIDEO_WIDTH,
                VIDEO_HEIGHT,
                dpi,
                DisplayManager.VIRTUAL_DISPLAY_FLAG_PRESENTATION,
                surface,
                null,
                null);
        EncoderThread encoderThread = new EncoderThread(encoder);
        encoderThread.setEncoderListener(this);
        encoder.start();
        encoderThread.start();
    }

    @Override
    public void onEncodeAFrame(byte[] frame)
    {
        liveNative.feedH264Data(frame);
    }
}
