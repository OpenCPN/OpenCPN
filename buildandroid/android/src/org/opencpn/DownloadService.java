package org.opencpn;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.TextView;
import android.content.Intent;
import android.net.NetworkInfo;
import android.net.ConnectivityManager;
import android.app.IntentService;
import android.os.ResultReceiver;
import java.io.BufferedInputStream;
import java.io.OutputStream;
import java.net.URLConnection;
import java.io.IOException;
import android.net.Uri;

public class DownloadService extends IntentService {
    public static final int UPDATE_PROGRESS = 8344;
    public static final int DOWNLOAD_DONE = 8345;
    public DownloadService() {
        super("DownloadService");
//        Log.i("OpenCPN", "DownloadService ctor ");
    }
    @Override
    protected void onHandleIntent(Intent intent) {
        String urlToDownload = intent.getStringExtra("url");
        String fileDestination = intent.getStringExtra("file");

        Uri fURI = Uri.parse(fileDestination);
        String file = "";
        try{
            file = fURI.getPath();
        }catch (Exception e) {
        }


        ResultReceiver receiver = (ResultReceiver) intent.getParcelableExtra("receiver");
//        Log.i("OpenCPN", "onHandleIntent " + file + receiver);

        long total = 0;
        int fileLength = -1;

        try {
            URL url = new URL(urlToDownload);
            URLConnection connection = url.openConnection();
            connection.connect();

            // this will be useful so that you can show a typical 0-100% progress bar
            fileLength = connection.getContentLength();

            // download the file
            InputStream input = new BufferedInputStream(connection.getInputStream());
            OutputStream output = new FileOutputStream(file);

            byte data[] = new byte[1024];
            int count;
            while ((count = input.read(data)) != -1) {
                total += count;
                // publishing the progress....
                Bundle resultData = new Bundle();

                int progress = (int) (total * 100 / fileLength);
                resultData.putInt("progress" , progress);
                resultData.putInt("sofar" , (int)total);
                resultData.putInt("filelength" , fileLength);

//                Log.i("OpenCPN", "RECEIVER SEND " + total + " " + fileLength + " " + progress);
                receiver.send(UPDATE_PROGRESS, resultData);
                output.write(data, 0, count);
            }

            output.flush();
            output.close();
            input.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        Bundle resultData = new Bundle();
        resultData.putInt("progress" ,100);
        resultData.putInt("sofar" , (int)total);
        resultData.putInt("filelength" , fileLength);
        receiver.send(DOWNLOAD_DONE, resultData);
    }
}































