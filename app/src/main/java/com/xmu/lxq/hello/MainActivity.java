package com.xmu.lxq.hello;

import android.content.pm.PackageManager;
import android.nfc.Tag;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.os.Environment;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.app.Activity;

public class MainActivity extends AppCompatActivity {
    private Button btn_emb;
    private Button btn_ext;
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE" };


    public static void verifyStoragePermissions(Activity activity) {

        try {
            //检测是否有写的权限
            int permission = ActivityCompat.checkSelfPermission(activity,
                    "android.permission.WRITE_EXTERNAL_STORAGE");
            if (permission != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE,REQUEST_EXTERNAL_STORAGE);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
        System.loadLibrary("x264");
    }
    public native void robust_embedding(String input,String output,String Message,double embrate,double qStep);
    public native void robust_extracting(String input,String Messageoutput,double embrate,double qStep);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        verifyStoragePermissions(this);
        setContentView(R.layout.activity_main);
        TextView tv = (TextView) findViewById(R.id.LinearLayout1);

        btn_emb = (Button) findViewById(R.id.btn_emb);
        btn_emb.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                String input= "/sdcard/videokit/cover/1.mp4";
                String output= "/sdcard/videokit/stego/1_stego.mp4";
                String Message = "/sdcard/videokit/message.txt";
                double embrate = 1;
                double qStep = 150;
                try{
                    robust_embedding(input,output,Message,embrate,qStep);
                }
                catch (StackOverflowError|Exception e)
                {
                    System.out.println(e.toString());
                }
                btn_emb.setText("Emb_Done!");
            }
        });

        btn_ext = (Button) findViewById(R.id.btn_ext);
        btn_ext.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                //String input= "/sdcard/videokit/1_stego.mp4";
                String input= "/sdcard/videokit/stego/1_stego.mp4";
                String Message = "/sdcard/videokit/message_ext.txt";
                double embrate = 1;
                double qStep = 150;
                try{
                    robust_extracting(input,Message,embrate,qStep);
                }
                catch (StackOverflowError|Exception e)
                {
                    System.out.println(e.toString());
                }
                btn_ext.setText("Ext_Done!");
            }
        });

    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */


}
