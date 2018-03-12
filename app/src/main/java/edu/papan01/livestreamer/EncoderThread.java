package edu.papan01.livestreamer;

import android.media.MediaCodec;
import android.util.Log;

import java.nio.ByteBuffer;

public class EncoderThread extends Thread{
    public interface EncoderListener {
        void onEncodeAFrame(byte[] frame);
    }

    private MediaCodec mediaCodec = null;
    private EncoderListener mListener = null;
    private static final int TIMEOUT = 60 * 1000;

    public EncoderThread(MediaCodec codec) {
        mediaCodec = codec;
    }

    public void setEncoderListener(EncoderListener listener) {
        this.mListener = listener;
    }

    @Override
    public void run() {
        boolean finished = false;
        MediaCodec.BufferInfo bufferInfo  = new MediaCodec.BufferInfo();
        while (!finished) {
            int inputBufferId  = mediaCodec.dequeueOutputBuffer(bufferInfo , TIMEOUT);

            if (inputBufferId  == MediaCodec.INFO_TRY_AGAIN_LATER)
            {
                Log.d("EncoderThread", "INFO TRY AGAIN LATER");
            }
            else if (inputBufferId  == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED)
            {
                Log.d("EncoderThread", "INFO OUTPUT FORMAT CHANGED");
            }
            else
            {
                ByteBuffer outputBuffer  = mediaCodec.getOutputBuffer(inputBufferId);

                outputBuffer.position(bufferInfo .offset);
                outputBuffer.limit(bufferInfo .offset + bufferInfo .size);

                byte[] srcBuffer = new byte[bufferInfo .size];
                outputBuffer.get(srcBuffer, 0, bufferInfo.size);
                mListener.onEncodeAFrame(srcBuffer);

                finished = (bufferInfo .flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0;
                mediaCodec.releaseOutputBuffer(inputBufferId, false);
            }
        }
    }
}
