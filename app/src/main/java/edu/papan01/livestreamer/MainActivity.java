package edu.papan01.livestreamer;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.projection.MediaProjectionManager;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Locale;

public class MainActivity extends AppCompatActivity {
    private TextView ip_address;
    public static final int PERMISSION_CODE = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button server_btn;
        server_btn = findViewById(R.id.server_btn);
        ip_address = findViewById(R.id.ip_address);

        server_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Toast.makeText(MainActivity.this,"START", Toast.LENGTH_SHORT).show();
                startRtspService();
            }
        });

        displayWifiInfo();
    }

    private void startRtspService() {
        MediaProjectionManager manager =
                (MediaProjectionManager) this.getSystemService(Context.MEDIA_PROJECTION_SERVICE);
        Intent intent = manager.createScreenCaptureIntent();
        startActivityForResult(intent, PERMISSION_CODE);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        handleRecordScreenRequest(requestCode, resultCode, data);
        super.onActivityResult(requestCode, resultCode, data);
    }

    private void handleRecordScreenRequest(int requestCode, int resultCode, Intent data) {
        if (requestCode != PERMISSION_CODE) return;
        if (resultCode != Activity.RESULT_OK) return;

        // start background service
        Intent serviceIntent = new Intent(this,ScreenService.class);
        serviceIntent.putExtra("resultCode", resultCode);
        serviceIntent.putExtra("data", data);
        startService(serviceIntent);
    }

    private String getWifiAddr() {
        WifiManager wifiManager = (WifiManager)
                this.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        WifiInfo info = wifiManager.getConnectionInfo();
        String ipaddress = null;
        if (info != null && info.getNetworkId() > -1){
            int i = info.getIpAddress();
            ipaddress = String.format(Locale.ENGLISH, "%d.%d.%d.%d", i & 0xff,
                    i >> 8 & 0xff, i >> 16 & 0xff, i >> 24 & 0xff);
        }
        return ipaddress;
    }

    private void displayWifiInfo() {
        String ipAddr = getWifiAddr();
        if(ipAddr != null)
           ip_address.setText(String.format("rtsp://%s:%d/papan01", ipAddr, 8554));
        else
            ip_address.setText("Need open the wifi");
    }

    // BroadcastReceiver that detects wifi state changements
    private final BroadcastReceiver mWifiStateReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {

        }
    };

    @Override
    public void onPause() {
        super.onPause();
        unregisterReceiver(mWifiStateReceiver);
    }

    @Override
    public void onResume() {
        super.onResume();
        registerReceiver(mWifiStateReceiver,
                new IntentFilter(WifiManager.NETWORK_STATE_CHANGED_ACTION));
    }
}
